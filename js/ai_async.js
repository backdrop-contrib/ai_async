/**
 * @file
 * Shared JS polling utility for ai_async background jobs.
 */

(function ($) {
  'use strict';

  window.AIAsync = window.AIAsync || {};

  /**
   * Polls the status endpoint for a background job until it completes.
   *
   * @param {string} jobId
   *   The job UUID returned by ai_async_submit().
   * @param {object} options
   *   - interval    {number}   Polling interval in ms. Default: 1000.
   *   - maxWait     {number}   Give up after this many ms. Default: 120000.
   *   - onComplete  {function} Called with the result object on success.
   *   - onError     {function} Called with an error string on failure.
   *   - onTimeout   {function} Called if maxWait is exceeded.
   */
  AIAsync.poll = function (jobId, options) {
    var settings = $.extend({
      interval:   1000,
      maxWait:    120000,
      onComplete: function () {},
      onError:    function () {},
      onTimeout:  function () {}
    }, options);

    var statusUrl = (Backdrop.settings.ai_async && Backdrop.settings.ai_async.statusUrl)
      ? Backdrop.settings.ai_async.statusUrl
      : '/ai/async/status/';

    var elapsed = 0;
    var timer   = null;

    function check() {
      $.ajax({
        url:      statusUrl + jobId,
        type:     'GET',
        dataType: 'json',
        success: function (data) {
          if (data.status === 'complete') {
            settings.onComplete(data.result);
          }
          else if (data.status === 'error') {
            settings.onError(data.error || Backdrop.t('An unknown error occurred.'));
          }
          else {
            // Still pending or processing — schedule next poll.
            schedule();
          }
        },
        error: function () {
          settings.onError(Backdrop.t('Could not reach the status endpoint.'));
        }
      });
    }

    function schedule() {
      elapsed += settings.interval;
      if (elapsed >= settings.maxWait) {
        settings.onTimeout();
        return;
      }
      timer = setTimeout(check, settings.interval);
    }

    // Start the first check immediately.
    check();

    // Return a cancel handle.
    return {
      cancel: function () {
        clearTimeout(timer);
      }
    };
  };

}(jQuery));
