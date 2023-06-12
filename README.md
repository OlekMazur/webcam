Small WebCam server	{#mainpage}
===================

A program which captures video frames from a V4L2 device (e.g. USB camera),
converts them to JPEG with minimal effort,
and serves the resulting stream of video data as multipart/x-mixed-replace
MIME type:
- either to a single HTTP client using built-in primitive HTTP server, or
- via CGI responder interface.

Alternatively, it can save each JPEG image to a separate file, or pass
them to stdout.

"Minimal effort" means the following:
- in case of a camera producing JPEG frames - no effort at all;
- in case of a camera producing MJPEG frames - adding missing chunk of data with Huffmann table;
- in case of cheapest cameras giving just YUV 4:2:2 packed frames - converting to JPEG using jpeglib (or libjpeg-turbo).

Program can also be compiled without YUV support, what allows linking without jpeglib.

Examples
--------
Serving video with built-in HTTP server on TCP port 44444, with just 5 FPS, using no more than 16 MB of buffers:
```
nph-webcam.cgi -v -o http -r 5 -m 16 -p 44444
```
Video can be seen in a web browser at http://localhost:44444 provided
you have a supported webcam plugged in and permissions of its /dev/videoX
node include *rw* for the user running this program.

Video can also be embedded inside a web page like this:
```
<img id="webcam" src="http://server:44444" alt="Video stream">
```

CGI output is enabled by default so the program can be directly used
as a CGI script (`nph-webcam.cgi`), for example with *lighttpd* configured
this way:
```
server.modules += ( "mod_cgi" )
cgi.execute-x-only = "enable"
cgi.assign = ( ".cgi"  => "", ".sh" => "" )
server.stream-response-body = 2
```
Parameters can be passed by a helper script `nph-webcam.sh` like this:
```
#!/bin/sh

exec "${0%.sh}.cgi" -w 1280 -h 720
```

Documentation
-------------

See [documentation].

Notice
------

webcam @copyright © 2023 Aleksander Mazur

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the [GNU General Public License]
along with this program.  If not, see <https://www.gnu.org/licenses/>.

[GNU General Public License]: LICENSE.md
[documentation]: https://olekmazur.github.io/webcam
