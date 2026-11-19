# AI Async

Run slow AI requests in a background job. Integrating modules can return a job
ID to the browser, then check for results while the job runs.

## Requirements

- PHP 8.2 or later
- `ai` module

## Installation

- Install this module using the official
  [Backdrop CMS instructions](https://backdropcms.org/user-guide/modules).

## Configuration

1. Enable the module.
2. Register allowed callbacks with `hook_ai_async_callbacks()`.
3. Submit jobs with `ai_async_submit()` and poll status with
   `ai_async_get_job()` or the built-in status endpoint. Browser polling
   requires the signed-in user who submitted the job.

## Documentation

Find setup, examples, and troubleshooting in the [project wiki](https://github.com/backdrop-contrib/ai_async/wiki).

## Issues

Bugs and feature requests should be reported in the
[Issue Queue](https://github.com/backdrop-contrib/ai_async/issues).

## Current Maintainer

[Justin Keiser](https://github.com/keiserjb)

## Credits

- Created for Backdrop CMS by [Justin Keiser](https://github.com/keiserjb).
- Developed with AI assistance.

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory
for complete text.
