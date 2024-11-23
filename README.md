# efbpad

A prototype terminal that was once seen working on a Kobo Clara BW.
Type with a bluetooth keyboard.

## Installation

 - First install Nickel
 - Run `make eink` to produce a package. Things are compiled through
   NiLuJe's `koxtoolchain` kobo env.
 - Include the produced `KoboRoot.tgz` in `/mnt/onboard/.kobo/Kobo`

To uninstall, remove `/mnt/onboard/.adds/fbpad/` and `/mnt/onboard/fonts/tf/`

## Project Structure, Credits
It can't be stated enough, others ran so that fbpad-eink could crawl,
This project is just other people's work thinly threaded together.

Broadly there are 3 components:
 - `efbpad.sh`: Startup/shutdown script, "created" by just paring down
   KOReader's `koreader.sh`.
 - `fbpad`: A framebuffer terminal emulator.
    Here there are patches calls to NiLuJe's FBInk library to execute
    screen refreshes.
    Notably, this isn't the first time someone has done this
    (see fbpad-eink).
 - `kbreader`: Under proper conditions keyboards appear in linux as
    event devices.
    When you start `fbpad` it's waiting for chars from stdin.
    `kbreader` acts as the interpreter to translate keystrokes into
    char strings. It writes its translations to stdout.
    Thus we can get `fbpad` to listen to the keyboard by running a
    piped command as so:
    `kbreader /dev/input/event3 | fbpad the_shell_cmd`.
    `kbreader` is spiritually identical to inkvt's onscreen keyboard,
    except it interprets key events rather than soft-keyboard touches.

The included font `regular.tf` was produced using fbpad_mkfn:

```
$ ./mkfn -h 48 -w 24 DejaVuSansMono.ttf:42 > regular.tf
```