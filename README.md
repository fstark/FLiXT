# FliXT
FliXT is a flim player for the IBM PC XT. It currently targets Hercules Graphics Plus video cards.

# How to develop

Best tool for development is to use dosbox (dosbox-x undex OSX)
Use the ``dosbox.sh`` script to launch the dev environment with the correct configuration file. The ``dosbox.conf`` assumes that the development files are in ``~/Development/FliXT``. Update accordingly.

Use ``Ctrl-F7`` to select a good color scheme (for whatever reason, mine is purple on blue by default...)

In dosbox, use the "DOIT.BAT" batch file to make a full build of ``FLIXT.EXE`` (I prefere doing full builds due to the flakines of dependency management/time management). Use the ``CPU/Turbo`` option to make the build instant.