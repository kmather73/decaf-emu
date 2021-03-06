#include "coreinit.h"
#include "coreinit_mutex.h"
#include "coreinit_scheduler.h"
#include "coreinit_thread.h"
#include "coreinit_internal_queue.h"
#include "common/decaf_assert.h"

namespace coreinit
{

using MutexQueue = internal::Queue<OSMutexQueue, OSMutexLink, OSMutex, &OSMutex::link>;

const uint32_t
OSMutex::Tag;

const uint32_t
OSCondition::Tag;


/**
 * Initialise a mutex structure.
 */
void
OSInitMutex(OSMutex *mutex)
{
   OSInitMutexEx(mutex, nullptr);
}


/**
 * Initialise a mutex structure with a name.
 */
void
OSInitMutexEx(OSMutex *mutex,
              const char *name)
{
   decaf_check(mutex);
   mutex->tag = OSMutex::Tag;
   mutex->name = name;
   mutex->owner = nullptr;
   mutex->count = 0;
   OSInitThreadQueueEx(&mutex->queue, mutex);
   MutexQueue::initLink(mutex);
}


static void
lockMutexNoLock(OSMutex *mutex)
{
   auto thread = OSGetCurrentThread();
   decaf_check(thread->state == OSThreadState::Running);

   while (mutex->owner) {
      if (mutex->owner == thread) {
         mutex->count++;
         return;
      }

      // Mark this thread as waiting on the mutex
      thread->mutex = mutex;

      // Promote mutex owner priority
      internal::promoteThreadPriorityNoLock(mutex->owner, thread->priority);

      // Wait for other owner to unlock
      internal::sleepThreadNoLock(&mutex->queue);
      internal::rescheduleSelfNoLock();

      // We are no longer waiting on the mutex
      thread->mutex = nullptr;
   }

   // Set current thread to owner of mutex
   mutex->count++;
   mutex->owner = thread;
   MutexQueue::append(&thread->mutexQueue, mutex);
   thread->cancelState |= OSThreadCancelState::DisabledByMutex;
}


/**
 * Lock the mutex.
 *
 * If no one owns the mutex, set current thread as owner.
 * If the lock is owned by the current thread, increase the recursion count.
 * If the lock is owned by another thread, the current thread will sleep until
 * the owner has unlocked this mutex.
 *
 * Similar to <a href="http://en.cppreference.com/w/cpp/thread/recursive_mutex/lock">std::recursive_mutex::lock</a>.
 */
void
OSLockMutex(OSMutex *mutex)
{
   internal::lockScheduler();
   decaf_check(mutex);

   // HACK: Naughty games not initialising mutex before using it.
   //decaf_check(mutex->tag == OSMutex::Tag);
   if (mutex->tag != OSMutex::Tag) {
      OSInitMutex(mutex);
   }

   internal::testThreadCancelNoLock();
   lockMutexNoLock(mutex);
   internal::unlockScheduler();
}


/**
 * Try to lock a mutex.
 *
 * If no one owns the mutex, set current thread as owner.
 * If the lock is owned by the current thread, increase the recursion count.
 * If the lock is owned by another thread, do not block, return FALSE.
 *
 * \return TRUE if the mutex is locked, FALSE if the mutex is owned by another thread.
 *
 * Similar to <a href="http://en.cppreference.com/w/cpp/thread/recursive_mutex/try_lock">std::recursive_mutex::try_lock</a>.
 */
BOOL
OSTryLockMutex(OSMutex *mutex)
{
   internal::lockScheduler();
   decaf_check(mutex);

   auto thread = OSGetCurrentThread();
   decaf_check(thread->state == OSThreadState::Running);

   internal::testThreadCancelNoLock();

   if (mutex->owner == thread) {
      mutex->count++;
      internal::unlockScheduler();
      return TRUE;
   } else if (mutex->owner) {
      internal::unlockScheduler();
      return FALSE;
   }

   // Set thread to owner of mutex
   mutex->count++;
   mutex->owner = thread;
   MutexQueue::append(&thread->mutexQueue, mutex);
   thread->cancelState |= OSThreadCancelState::DisabledByMutex;

   internal::unlockScheduler();
   return TRUE;
}


/**
 * Unlocks the mutex.
 *
 * Will decrease the recursion count, will only unlock the mutex when the
 * recursion count reaches 0.
 * If any other threads are waiting to lock the mutex they will be woken.
 *
 * Similar to <a href="http://en.cppreference.com/w/cpp/thread/recursive_mutex/unlock">std::recursive_mutex::unlock</a>.
 */
void
OSUnlockMutex(OSMutex *mutex)
{
   internal::lockScheduler();
   decaf_check(mutex);
   decaf_check(mutex->tag == OSMutex::Tag);

   auto thread = OSGetCurrentThread();
   decaf_check(thread->state == OSThreadState::Running);
   decaf_check(mutex->owner == thread);

   // Decrement the mutexes lock count
   mutex->count--;

   // If we still own the mutex, lets just leave now
   if (mutex->count > 0) {
      internal::unlockScheduler();
      return;
   }

   // Remove mutex from thread's mutex queue
   MutexQueue::erase(&thread->mutexQueue, mutex);

   // Clear the mutex owner
   mutex->owner = nullptr;

   // If we have a promoted priority, reset it.
   if (thread->priority < thread->basePriority) {
      thread->priority = internal::calculateThreadPriorityNoLock(thread);
   }

   // Clear the cancelState flag if we don't have any more mutexes locked
   if (!thread->mutexQueue.head) {
      thread->cancelState &= ~OSThreadCancelState::DisabledByMutex;
   }

   // Wakeup any threads trying to lock this mutex
   internal::wakeupThreadNoLock(&mutex->queue);

   // Check if we are meant to cancel now
   internal::testThreadCancelNoLock();

   // Reschedule everyone
   internal::rescheduleAllCoreNoLock();

   // Unlock our scheduler and continue
   internal::unlockScheduler();
}


/**
 * Initialise a condition variable structure.
 */
void
OSInitCond(OSCondition *condition)
{
   OSInitCondEx(condition, nullptr);
}


/**
 * Initialise a condition variable structure with a name.
 */
void
OSInitCondEx(OSCondition *condition, const char *name)
{
   decaf_check(condition);
   condition->tag = OSCondition::Tag;
   condition->name = name;
   OSInitThreadQueueEx(&condition->queue, condition);
}


/**
 * Sleep the current thread until the condition variable has been signalled.
 *
 * The mutex must be locked when entering this function.
 * Will unlock the mutex and then sleep, reacquiring the mutex when woken.
 *
 * Similar to <a href="http://en.cppreference.com/w/cpp/thread/condition_variable/wait">std::condition_variable::wait</a>.
 */
void
OSWaitCond(OSCondition *condition,
           OSMutex *mutex)
{
   internal::lockScheduler();
   decaf_check(condition);
   decaf_check(condition->tag == OSCondition::Tag);
   decaf_check(mutex);
   decaf_check(mutex->tag == OSMutex::Tag);

   auto thread = OSGetCurrentThread();
   decaf_check(thread->state == OSThreadState::Running);
   decaf_check(mutex->owner == thread);

   // Save the count and then unlock the mutex
   auto mutexCount = mutex->count;
   mutex->count = 0;
   mutex->owner = nullptr;

   // Remove mutex from thread's mutex queue
   MutexQueue::erase(&thread->mutexQueue, mutex);

   // If we have a promoted priority, reset it.
   if (thread->priority < thread->basePriority) {
      thread->priority = internal::calculateThreadPriorityNoLock(thread);
   }

   // Wake anyone waiting on the mutex
   internal::disableScheduler();
   internal::wakeupThreadNoLock(&mutex->queue);
   internal::rescheduleAllCoreNoLock();
   internal::enableScheduler();

   // Sleep on the condition
   internal::sleepThreadNoLock(&condition->queue);
   internal::rescheduleSelfNoLock();

   // Relock the mutex
   lockMutexNoLock(mutex);
   mutex->count = mutexCount;

   internal::unlockScheduler();
}


/**
 * Will wake up any threads waiting on the condition with OSWaitCond.
 *
 * Similar to <a href="http://en.cppreference.com/w/cpp/thread/condition_variable/notify_all">std::condition_variable::notify_all</a>.
 */
void
OSSignalCond(OSCondition *condition)
{
   decaf_check(condition);
   decaf_check(condition->tag == OSCondition::Tag);
   OSWakeupThread(&condition->queue);
}


void
Module::registerMutexFunctions()
{
   RegisterKernelFunction(OSInitMutex);
   RegisterKernelFunction(OSInitMutexEx);
   RegisterKernelFunction(OSLockMutex);
   RegisterKernelFunction(OSTryLockMutex);
   RegisterKernelFunction(OSUnlockMutex);
   RegisterKernelFunction(OSInitCond);
   RegisterKernelFunction(OSInitCondEx);
   RegisterKernelFunction(OSWaitCond);
   RegisterKernelFunction(OSSignalCond);
}

namespace internal
{

void
unlockAllMutexNoLock(OSThread *thread)
{
   while (auto mutex = thread->mutexQueue.head) {
      // Remove this mutex from our queue
      MutexQueue::erase(&thread->mutexQueue, mutex);

      // Release this mutex
      mutex->count = 0;
      mutex->owner = nullptr;

      // Wakeup any threads trying to lock this mutex
      internal::wakeupThreadNoLock(&mutex->queue);
   }
}

} // namespace internal

} // namespace coreinit
