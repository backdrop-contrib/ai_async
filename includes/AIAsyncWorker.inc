<?php


/**
 * @file
 * Background job processor for ai_async.
 */

class AIAsyncWorker {

  /**
   * Processes a single pending job by job ID and token.
   *
   * Performs an atomic pending→processing transition so concurrent requests
   * cannot double-process the same job.
   *
   * @param string $job_id
   * @param string $token
   *
   * @return bool
   *   TRUE if the job was processed, FALSE if skipped (not pending / bad token).
   */
  public function process($job_id, $token) {
    $job = ai_async_get_job($job_id);
    if (!$job) {
      return FALSE;
    }

    // Prefer constant-time comparison when available.
    if (function_exists('hash_equals')) {
      $valid = hash_equals($job->token, $token);
    }
    else {
      $valid = ($token === $job->token);
    }

    if (!$valid) {
      watchdog('ai_async', 'Invalid token for job @id.', ['@id' => $job_id], WATCHDOG_WARNING);
      return FALSE;
    }

    // Atomic transition: only proceed if still pending.
    $updated = db_update('ai_async_jobs')
      ->fields(['status' => 'processing', 'updated' => REQUEST_TIME])
      ->condition('job_id', $job_id)
      ->condition('status', 'pending')
      ->execute();

    if (!$updated) {
      // Already processing or finished — skip silently.
      return FALSE;
    }

    $this->run($job);
    return TRUE;
  }

  /**
   * Runs the job callback and stores the result or error.
   */
  protected function run(AIAsyncJob $job) {
    $allowed = ai_async_get_allowed_callbacks();
    if (!isset($allowed[$job->callback])) {
      $this->fail($job->job_id, 'Callback ' . $job->callback . ' is not registered.');
      return;
    }

    if (!function_exists($job->callback)) {
      $this->fail($job->job_id, 'Callback function ' . $job->callback . ' does not exist.');
      return;
    }

    try {
      $data = $job->getData();
      $result = call_user_func($job->callback, $data);

      db_update('ai_async_jobs')
        ->fields([
          'status' => 'complete',
          'result' => json_encode($result),
          'updated' => REQUEST_TIME,
        ])
        ->condition('job_id', $job->job_id)
        ->execute();
    }
    catch (\Exception $e) {
      $this->fail($job->job_id, $e->getMessage());
      watchdog('ai_async', 'Job @id failed: @msg', [
        '@id' => $job->job_id,
        '@msg' => $e->getMessage(),
      ], WATCHDOG_ERROR);
    }
    catch (Error $e) {
      $this->fail($job->job_id, $e->getMessage());
      watchdog('ai_async', 'Job @id failed: @msg', [
        '@id' => $job->job_id,
        '@msg' => $e->getMessage(),
      ], WATCHDOG_ERROR);
    }
  }

  /**
   * Marks a job as errored.
   */
  protected function fail($job_id, $message) {
    db_update('ai_async_jobs')
      ->fields([
        'status' => 'error',
        'error' => $message,
        'updated' => REQUEST_TIME,
      ])
      ->condition('job_id', $job_id)
      ->execute();
  }

}
