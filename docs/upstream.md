# Provenance and initial audit

Imported 2026-09-11 from https://github.com/JosephHewitt/wardriver_rev3

Pinned Git tree: `6974f57e2b1585b3c2101ac53eec509ecd527424`.
This is a tree identifier, not a release tag. Upstream main identifies itself as 1.3.0b2 and is a testing branch.

Imported: A/A.ino, B/B.ino, LICENSE, boards.txt, libraries.txt.
Original README retained at upstream-readme.md. The original release-publishing workflow was not imported; our workflow only compiles.

## Modified files
A/A.ino: development version label; BW16 and Fahrenheit defaults; WiGLE credential trim on load/save; file size equality fix; zeroed certificate buffers; upstream OTA opt-out forced at startup.
B/B.ino: BW16 default enabled.

## WiGLE investigation
The source sends `Authorization: Basic <stored value>`. The field expects the full encoded credential, not an isolated API token. Input was URL-decoded and saved without whitespace normalization.

The UI's generic login-failed indication is based on missing username data. A missing CA file, TLS problem, response parsing problem, or authentication rejection can all leave the username unset. Do not treat this indication as proof that a key is invalid.

Two PEM arrays were uninitialized; the read leaves unused bytes unspecified, while setCACert requires a terminated string. These buffers are now zeroed. Certificate freshness and short-read handling still need validation.

History lookup previously checked that stored file size was nonzero rather than equal to the requested size. Corrected, but file ID plus size is still not a collision-resistant identity. The new queue should use a content digest.

Legacy credential submission uses a GET form. Replace it with a bounded POST handler before the redesigned credential UI is considered complete. Do not add real credentials or collected logs to this public repository.

## Remaining upstream behavior
Existing upload transport, SD CA handling, OTA routes and configuration UI remain largely upstream code. Automatic OTA selection is disabled; OTA UI cleanup is pending. Do not use manual upstream OTA to update this fork.

Hardware, real-service uploads, and failure recovery are not validated by this import.


## BW16 companion
Imported without changes from CoD-Segfault/BW16-Open-AT, blob 2ba595f33b1c9ddda8975d293ba3288cb62537d8. See BW16-Open-AT/README.md. Not compiled or hardware-tested here.
