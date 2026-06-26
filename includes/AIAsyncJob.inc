<?php


/**
 * @file
 * Value object representing a single async AI job.
 */

class AIAsyncJob {

  public $job_id;
  public $callback;
  public $data;
  public $status;
  public $result;
  public $error;
  public $token;
  public $uid;
  public $created;
  public $updated;

  /**
   * Constructs a job from a database row stdClass.
   */
  public function __construct($row) {
    foreach ((array) $row as $key => $value) {
      $this->{$key} = $value;
    }
  }

  /**
   * Returns the unserialized input data.
   */
  public function getData() {
    if (!$this->data) {
      return [];
    }
    // allowed_classes => FALSE turns serialized objects into __PHP_Incomplete_Class
    // rather than returning FALSE, so a top-level is_array() check is not enough.
    $result = unserialize($this->data, ['allowed_classes' => FALSE]);
    if (!is_array($result)) {
      return [];
    }
    $has_objects = FALSE;
    array_walk_recursive($result, function ($value) use (&$has_objects) {
      if (is_object($value)) {
        $has_objects = TRUE;
      }
    });
    return $has_objects ? [] : $result;
  }

  /**
   * Returns the decoded result, or NULL if not yet complete.
   */
  public function getResult() {
    return $this->result ? json_decode($this->result, TRUE) : NULL;
  }

  /**
   * Whether the job has finished (successfully or not).
   */
  public function isFinished() {
    return in_array($this->status, ['complete', 'error'], TRUE);
  }

}
