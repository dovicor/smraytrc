# smraytrc
Sun-mirror ray tracer

smraytrc: Sun-Mirror ray-tracer

A simple 2D ray-tracer targeted for the specialized case for the Sun reflecting
off a mirror. As opposed to other ray-tracers, this accounts for the angular
width of the Sun - and its placement at an infinite distance. Further, many
other ray-tracers ignore the intensity of reflected illumination.

A command-line programa written in C++, executable in Windows (with or without
cygwin) or Linux.

Output is vector graphics - using SVG - intended to be rendered in a browser.

The Makefile contains numerous test-cases.


Building/Installing
The source is a single C++ file. The Makefile builds it to an executable, given
Cygwin, MinGW or Linux. The executable can then be executable from the command
line (no installation necessary).
Once built:
    $ ./smraytrc.exe -help   # should generate the usage message (further below).
    $ ./smraytrc.exe -test   # Runs a series of internal tests.
    $ ./smraytrc -r 10 -mna 280 -mxa 300 -sa 270 -sa 320  -svg output_first.svg # Creates an SVG file
The 3rd command-line creates an SVG file. Load that into a browser to observe the image. There
are several possible ways to do this.
    From cygwin: $ cygstart chrome output_first,svg
Or enter the pathname of the file in your browser's URL.
Here's an example: file:///C:/cygwin64/home/Don/src/smraytrc/output_first.svg
That example shows a small section of a circular mirror (20 degree arc-length). The incident
rays from the Sun are shown in yellow and orange - using the default 0.5 degree angular width for the sun.
The reflection of those rays are rendered in green and blue - continuing off-screen. Only a few
points along the mirror are selected to be rendered - to the two end-points and the middle.


Usage:
$ ./smraytrc.exe -help
Usage: C:\cygwin64\home\Don\src\smraytrc\smraytrc.exe [-next | -iterate] [-r ...] [-d ...] [-s[aA] ...] [-sw <value>] [-svg [<filename>]] [-csv] [-pupil] [-animate]
        Runs a reverse ray-tracing for a system involving the sun, a (convex) mirror and (sometimes) an observer.
        -next: may be used to specify a number of test-conditions. Separates the -r, -d, -sa  (and -sA) arguments. May be used multiple
                times to specify multiple test-conditions. Such as C:\cygwin64\home\Don\src\smraytrc\smraytrc.exe -r 1 -d 1.1 -sA 10 -next -r 1 -d 1.1 -sA 20 -next -r 1 -d 1.1 -sA 30
                Not compatible with the -iterate argument.
        -iterate: Use before -r, -d and/or -sa (and -sA) arguments. In this case each of these arguments may be followed by a series of values:
                <value1>: Only the single value is used in the iteration.
                <value1> <value2>: Iterates twice, first with value1 and then with value2
                <value1> <value2> <value3> (if value2>value3): Iterate from value1 to <=value2, incrementing by value3.
                <value1> <value2> <value3> ... (if value2<value3): taken as a series of values.
                Note that -iterate and -next are not compatible with each other.
        -r <value> [...]: Defines the radius if the mirror (defaults to 1). See above comments regarding -next and -iterate.
        -d <value> [...]: Defines the distance from the observer to the center-of-curvature of the mirror. Must be >radius. See
                above comments.
        -sa <value> [...]: Defines the the angle of the sun's rays (i.e. 0 is horizontal to the left, 270 vertically down).
        -sA <value> [...]: An alterative to -sa - defines the sun's altitude (is 180 more/less than -sa): 90 is vertically down.
        -sw <value>: Defines the angular width of the sun in degrees. Defaults to 0.5. The above comments are not applicable.
        -csv: generates results in a comma-separated-values format on standard-output.
        -svg <filename>: generates SVG graphics in the indicated filename. Typically observer in a browser.
        -animate: Adds animation to the SVG (per the test-cases identified with -next or -iterate).
        -pupil: (experimental) - perform and report on the the entrance pupil calculations.



Copyright Don Organ 2019. All rights reserved.

