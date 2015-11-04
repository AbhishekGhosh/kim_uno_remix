This directory was intended to be a console-based version of the KIM Uno.

I was modifying it to build without touching the arduino-specific
source code and so I created the "ardsim" directory to contain all
of the header files and classes/functions that the code might need
(ie Serial, EEPROM libraries.)  It was mostly complete, but I decided
to take things in a different direction and create the QT version
instead.

As of 11/1, the interfaces have changed enough that this probably
doesn't compile or work properly anymore.

I'm leaving this here because it may be of use to someone, maybe?
-Scott

--------------------------------------------------------------------------------
License: (MIT)
--------------------------------------------------------------------------------

Copyright (c) 2015 Scott Lawrence

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject
to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

