# Camera-tested 7D2.112 DEFOE RAW experiment

**Experimental reference branch, not an upstream contribution submission.**
Under [`LLM_POLICY.md`](../../../LLM_POLICY.md), Magic Lantern will not accept
substantial directly LLM-generated code. This branch exceeds that allowance.
The local recorder changes, helper headers, FPS changes, tests and these notes
were directly generated or modified by an LLM; they are not presented as a
human-authored implementation. The user performed the physical camera tests.
Existing upstream code remains attributed and licensed by its original authors.
Upstream eligibility would require a maintainer-approved path or a contributor's
own understood implementation; relabeling this code as merely LLM-guided does
not satisfy that policy. No upstream pull request is being submitted.

This branch preserves the recorder source used for the one-minute and repeated
ISO-5000 tests on a Canon 7D Mark II, firmware 1.1.2. It is based on upstream
`dev` at `142472e95054d73640f50789ac0fd66325499d82`.

There is no new upstream integration here. The recorder and its three local
headers match the archived tested source; Git normalizes Windows line endings.
The earlier source-only integration was superseded. Existing inline checks and
setup/probe modes are retained because they are part of that exact source.
Standalone research modules, investigation logs, media, ROM dumps, binaries and
machine-specific tools are excluded.

## Required changes

The first commit preserves the tested 1832-wide active-area transfer and FPS
prerequisites. `active_7d2.h` defines the measured source layout and copy bounds;
`fast_probe_7d2.h` constructs the descriptor transfers used by this recorder.
The 7D2-only FPS reporting change uses Canon's nominal movie rate where this
port's uncalibrated timer otherwise gives incorrect metadata and fails the
correction test's 23.976-fps readiness check. Other modes retain the timer path.

The second commit adds the exact tested capture-point lifecycle and 60-second
trial. The original source is 14 (RSHD). On 7D2.112, native Thumb function
`FE2B196B(4)` selects source 18 (DEFOE). The selector variable is `0002D7D0`;
the mux is observed through `shamem_read(D0037014) & 63`. The setter fingerprint
at `FE2B196A` is `24 49 0E 28 10 B5 2F D2`. These are firmware-specific values,
not a generic DIGIC6 interface. `shamem_read` observes Canon's software register
shadow rather than independently acknowledging live hardware.

The recording task validates the known profile, selects source 18 and waits for
three VSYNC counts and the expected mux. The original selector is restored on
cleanup and its mux is checked after Live View resumes. Failed verification
latches a restart requirement. No source selection or sleeping is added to the
VSYNC callback. Correction retains the known native 14-bit packed Bayer profile;
it does not alter ADC precision or add software 14-to-12/10-bit conversion.

## Tested configuration and results

Use normal Canon 1080 movie mode at 23.98 fps and 1× Live View. Under RAW video's
advanced settings select **7D2 startup test → 1832 manual RAW**, **7D2 800-row
trial → 1832x800**, **7D2 capture test → Corrected 60s**, and native 14-bit format.
Pre-recording, triggers, proxy and card spanning must be OFF. These experimental
options default OFF; no saved camera configuration is distributed. Close the
menu before recording. Manual stop is supported; the automatic limit is 1,439
frames, approximately 60.018 seconds. Do not use other built-in probe modes as
part of this recording procedure.

The native source is 1984×1068 with 3472 bytes/row. The tested recording is
1832×800 at source origin (144,144), uncompressed 14-bit at nominal 23.976 fps.
Before/after snapshots found 689 persistent zero-valued sites before source
switching and none of those sites at zero afterward. This is consistent with
Canon defect correction occurring before the later capture point; the native
algorithm itself has not been fully characterized.

The one-minute run saved 1,439 frames. Three subsequent ISO-5000 clips saved
116, 1,394 and 176 frames with manual stop/start on the same boot. All 858,174
samples at the 509 mapped sites within those clips' saved window were nonzero;
source restoration was verified after each stop. No PC pixel repair or denoising
was applied to these source-payload measurements. The recorded ISO metadata
does not independently certify manual/auto ISO control throughout every frame.

These results support correction of the measured zero/invalid-site map, not
removal of every hot pixel or noise. Packed format, Bayer geometry and metadata
were checked; independent sensor precision, calibrated color/linearity and added
latency were not established. Other readouts, magnification, cameras, long/split
recordings and arbitrary error/mode transitions remain outside the tested scope.
The timer-overlay fix and later lossless/low-bit experiments are not included.

The tested module's SHA256 was
`cc7f99517dd7107418c57edc9ee1460f60ae5b502aa59fd98dacceb60bbe7e4a`.
This identifies the archived camera-tested binary, not a newly signed release.
Rebuilding this branch reproduced the archived module's code, constants, data,
configuration and relocations byte-for-byte. The only differing section was
`.module_strings`, containing the newly generated build timestamp. The rebuilt
module SHA256 is
`d8c24db9bef8eb850bde64f4d6f625f80bad9cb0f471d36e98cb28f46621f4e7`.
The original camera runs and limits are summarized in the separate
[research archive's validation notes](https://github.com/navy-exe/7d2-raw-pixel-fix/blob/main/docs/VALIDATION.md).

## Validation and contribution scope

Focused tests in `tests/` cover the actual lifecycle helper, active-area bounds,
descriptor construction and the actual FPS reporting function. With a native C
compiler, build/run each `tests/*.c` file using `-std=c99 -O2 -Wall -Wextra
-Werror`; run `python3 tests/test_fps_reporting.py` with `cc` on PATH or set `CC`.
The test directory is relative to `modules/raw_video/mlv_lite`. These tests mock
hardware boundaries and do not replace the camera evidence above.

Before committing, clean modules as described in
[`CONTRIBUTING.md`](../../../CONTRIBUTING.md), then build the module and
`platform/7D2.112`. This branch retains the tested implementation rather than
reformatting or removing its embedded checks. It is experimental review work,
not a claim that the complete camera-build matrix or every hardware transition
has been validated.

The module and complete 7D2.112 platform builds passed after cleaning modules,
using ARM GCC 13.3.1 with `FATAL_WARNINGS=y` and `CONFIG_QEMU=n`. Focused host
tests passed, including 31 lifecycle cases, and the original 22-case compiled
ARM harness passed with camera APIs mocked. The full platform build retains
warnings from unmodified upstream code/tools; no new changed-code compiler
warnings were observed. The complete camera-build matrix was not run.

Thanks to **names_are_hard**, **Petars23_** and the Magic Lantern developers for
their knowledge, guidance and existing port/capture infrastructure. This is
AI-assisted work; these credits do not imply their approval of the patch.
