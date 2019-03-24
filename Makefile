#  Copyright (c) Don Organ 2018, 2019
# All rights reserved.
#


# Makefile contains rules for building the C++ source to an executable
# (using g++ for Linux or Cygwin, or MinGW on Windows).


ifeq ($(OS),Windows_NT)
	ifeq ($(shell uname -o), Cygwin)
		CC=x86_64-w64-mingw32-g++ -std=c++11 -g  -static # 64 bit C++
		ShowInBrowser=cygstart chrome
	else
	endif
else # Assuming Linux
		CC=g++ -std=c++11 -g  -Wno-psabi -Werror
		ShowInBrowser=echo 
endif

#CC=g++ -std=c++11 -g  -Wno-psabi -Werror
#CC=i686-w64-mingw32-gcc # 32 bit C
#CC=i686-w64-mingw32-g++ -std=c++11 -g# 32 bit C++
#CC=x86_64-w64-mingw32-gcc # 64 bit C
#CC=x86_64-w64-mingw32-g++ -std=c++11 -g  -static # 64 bit C++
#CC=x86_64-w64-mingw32-g++ -std=c++11 -g -m32

#ShowInBrowser=cygstart chrome
#ShowInBrowser=DISPLAY=:0 chromium-browser -start-maximized
#ShowInBrowser=echo 


default: smraytrc

all: test ConvexTests ConcaveTests examples
examples: example1 example2 example3 example4 example5 example6 example7


ConcaveTests: first ccv_f1 ccv_f2 ccv_f3 ccv_f4a ccv_f5a ccv_r6 ccv_r7 ccv_r8
ConvexTests:   cvx_f1 cvx_f2 cvx_f3 cvx_f4 cvx_f5 cvx_f6

LongRuntimeTest: ccv_iterate ccv_blur ccv_width ccv_focal ccv_zz ccv_svg ccv_animate test ccv_rv

both: smraytrc

smraytrc: smraytrc.cpp Makefile
	${CC} -o smraytrc smraytrc.cpp
#	g++ -std=c++11 -ggdb -o smraytrc smraytrc.cpp



first: smraytrc
	./smraytrc -r 10 -mna 280 -mxa 300 -sa 320  -svg output_$@.svg
	${ShowInBrowser}  output_$@.svg

ccv_f1: smraytrc
	./smraytrc -concave \
	       	-r 30 -sa 342 -sw 0.5 -mna 250 -mxa 550  \
		-nr 11 -svg output_$@.svg -report -title "$@: Forward, multiple internal reflections."
	${ShowInBrowser}  output_$@.svg

ccv_f2: smraytrc
	./smraytrc -concave \
			-screen 20 -9.8   10 23 \
			-stencil 10 10  10 -12 \
			-stencil 10 -15  10 -28 \
	       	-r 30 -sa 290 -sw 0.5 -mna 250 -mxa 260  \
		-nr 5 -svg output_$@.svg -report -title "$@: Forward, narrow mirror, stencil, screen."
	${ShowInBrowser}  output_$@.svg

ccv_f2R: smraytrc
	./smraytrc -concave \
			-screen 20 -9.8   10 23 \
			-stencil 10 10  10 -12 \
			-stencil 10 -15  10 -28 \
	       	-r 30 -sa 290 -sw 0.5 -mna 250 -mxa 260  \
			-reverse \
		-nr 5 -svg output_$@.svg -report -title "$@: Forward, narrow mirror, stencil, screen. Reverse."
	${ShowInBrowser}  output_$@.svg


ccv_f3: smraytrc
	./smraytrc -concave \
			-screen 20 -9.8   18 -5 \
			-stencil 10 10  10 -12 \
			-stencil 10 -15  10 -28 \
	       	-r 30 -sa 290 -sw 0.5 -mna 250 -mxa 260  \
			-iterate -sa 298 278 -0.10 \
		-nr 5 -svg output_$@.svg -report -title "$@: Forward, animated, narrow mirror, stencil screen." -animate -interval 100
	${ShowInBrowser}  output_$@.svg

ccv_f4a: smraytrc
	./smraytrc -concave  \
	    	-r 30 -mna 280 -mxa 300 -sw 0.5 \
			-iterate -sa 360 180 -0.25 \
	       -svg  output_$@.svg -pupil -animate -debug 0 -title "$S: Forward, animated"
	${ShowInBrowser}  output_$@.svg


ccv_f5a: smraytrc
	./smraytrc -concave  \
	    	-r 30 -mna 230 -mxa 530 -sw 0.5 -nr 7 \
			-iterate -sa 360 300 -1 \
	       -svg  output_$@.svg -pupil -animate -debug 0 -title "$@: Forward, animated"
	${ShowInBrowser}  output_$@.svg




ccv_r6: smraytrc
	./smraytrc -concave \
			-screen 20 -9.8   18 -5 \
			-stencil 10 6  10 -12 \
			-stencil 10 -15  10 -18 \
	       	-r 30 -sw 0.5 -mna 180 -mxa 360  \
			-iterate -sa 240 350 1 \
			-reverse \
		-svg output_$@.svg -report -title "$@: Reverse, animated." -animate
	${ShowInBrowser}  output_$@.svg

ccv_r7: smraytrc
	./smraytrc -concave \
			-screen 20 -9.8   18 -5 \
			-stencil 10 6  10 -12 \
			-stencil 10 -15  10 -18 \
	       	-r 30 -sw 0.5 -mna 180 -mxa 360  \
			-iterate -sa 300 320 2 \
			-reverse \
		-svg output_$@.svg -report -title "$@: Reverse, interated, cluttered."
	${ShowInBrowser}  output_$@.svg



ccv_r8: smraytrc
	./smraytrc -concave \
			-screen 20 -9.8   18 -5 \
			-stencil 10 6  10 -12 \
			-stencil 10 -15  10 -18 \
	       	-r 30 -sw 0.5 -mna 220 -mxa 270  \
			-iterate -sa 240 320 1 \
			-reverse \
		-svg output_$@.svg -report -title "$@: Reverse, animated, narrower mirror." -animate
	${ShowInBrowser}  output_$@.svg



test: cvx_test ccv_test


cvx_f1: smraytrc
	./smraytrc -convex      -r 1 -d 3 -sa 181 -sw 0.5 \
		-next -r 1 -d 3 -sa 185 -sw 0.5 \
		-next -r 1 -d 3 -sa 190 -sw 0.5 \
		-next -r 1 -d 3 -sa 200 -sw 0.5 \
		-next -r 1 -d 3 -sa 210 -sw 0.5 \
		-next -r 1 -d 3 -sa 220 -sw 0.5 \
		-next -r 1 -d 3 -sa 230 -sw 0.5 \
		-next -r 1 -d 3 -sa 240 -sw 0.5 \
		-next -r 1 -d 3 -sa 250 -sw 0.5 \
		-next -r 1 -d 3 -sa 260 -sw 0.5 \
		-next -r 1 -d 3 -sa 269 -sw 0.5 \
		-next -r 1 -d 3 -sa 270 -sw 0.5 \
		-next -r 1 -d 3 -sa 271 -sw 0.5 \
		-next -r 1 -d 3 -sa 280 -sw 0.5 \
		-next -r 1 -d 3 -sa 290 -sw 0.5 \
		-next -r 1 -d 3 -sa 300 -sw 0.5 \
		-next -r 1 -d 3 -sa 310 -sw 0.5 \
		-next -r 1 -d 3 -sa 320 -sw 0.5 \
		-next -r 1 -d 3 -sa 330 -sw 0.5 \
		-next -r 1 -d 3 -sa 340 -sw 0.5 \
		-next -r 1 -d 3 -sa 341 -sw 0.5 \
		-next -r 1 -d 3 -sa 342 -sw 0.5 \
		-next -r 1 -d 3 -sa 343 -sw 0.5 \
		-next -r 1 -d 3 -sa 344 -sw 0.5 \
		-next -r 1 -d 3 -sa 345 -sw 0.5 \
		-next -r 1 -d 3 -sa 350 -sw 0.5 \
		-next -r 1 -d 3 -sa 355 -sw 0.5 \
		-next -r 1 -d 3 -sa 359 -sw 0.5 \
		-svg  output_$@.svg
	${ShowInBrowser}  output_$@.svg



ZZ1: smraytrc
	./smraytrc -convex \
      -r 1 -d 5 -sa 80 -sw 0.5 \
		-next -r 1 -d 5 -sa 260 -sw 0.5 \
		-svg  output_$@.svg -title "$@: Debug - a few points of cvx_f1"
	${ShowInBrowser}  output_$@.svg

ZZ2: smraytrc
	./smraytrc -convex  \
	    	-r 1 \
			-d 5 \
	       	-sa 80 \
	       	-sw 0.5 \
		-svg  output_$@.svg -pupil -title "$@: Debug - one point in cvx-iterate"
	${ShowInBrowser}  output_$@.svg
#	       	-d 1.01 1.03 1.1 1.3 2 3 5 10 20 30 50 100 200 300 500 1000 2000 3000 5000 10000 \

cvx_f4: smraytrc
	./smraytrc -convex  -iterate \
	    	-r 1 \
	       	-d 1.01 1.03 1.1 1.3 2 3 5 10 20 30 50 100 200 300 500 1000 2000 3000 5000 10000 \
	       	-sa 0 180 10 \
	       	-sw 0.5 \
		-svg  output_$@.svg -pupil -title "$@: Convex iterate"
	${ShowInBrowser}  output_$@.svg

cvx_f6: smraytrc
	./smraytrc -convex -iterate \
	    	-r 1 \
	       	-d 1.01 1.03 1.1 1.3 2 3 5 10 20 30 50 100 200 300 500 1000 2000 3000 5000 10000 \
	       	-sa 180 360 0.5 \
	       	-sw 0.5 \
		-svg  output_$@.svg -pupil -csv2 sun_a distance pupil
	${ShowInBrowser}  output_$@.svg

cvx_f5: smraytrc
	./smraytrc -convex      -r 1 -d 1.01 -sA 0 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 2.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 7.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 10 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 12.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 15 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 17.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 20 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 22.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 25 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 27.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 30 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 32.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 35 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 37.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 40 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 42.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 45 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 47.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 50 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 52.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 55 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 57.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 60 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 62.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 65 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 67.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 70 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 72.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 75 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 77.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 80 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 82.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 85 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 87.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 90 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 92.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 95 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 97.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 100 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 102.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 105 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 107.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 110 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 112.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 115 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 117.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 120 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 122.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 125 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 127.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 130 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 132.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 135 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 137.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 140 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 142.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 145 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 147.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 150 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 152.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 155 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 157.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 160 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 162.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 165 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 167.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 170 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 172.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 175 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 177.5 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 179 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 180 -sw 0.5 \
	       -next -r 1 -d 1.01 -sA 181 -sw 0.5 \
-next -r 1 -d 1.1 -sA 0 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 2.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 7.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 10 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 12.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 15 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 17.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 20 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 22.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 25 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 27.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 30 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 32.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 35 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 37.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 40 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 42.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 45 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 47.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 50 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 52.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 55 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 57.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 60 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 62.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 65 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 67.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 70 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 72.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 75 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 77.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 80 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 82.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 85 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 87.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 90 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 92.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 95 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 97.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 100 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 102.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 105 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 107.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 110 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 112.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 115 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 117.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 120 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 122.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 125 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 127.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 130 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 132.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 135 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 137.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 140 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 142.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 145 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 147.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 150 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 152.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 155 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 157.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 160 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 162.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 165 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 167.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 170 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 172.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 175 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 177.5 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 179 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 180 -sw 0.5 \
	       -next -r 1 -d 1.1 -sA 181 -sw 0.5 \
	-next -r 1 -d 1.5 -sA 0 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 2.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 7.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 10 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 12.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 15 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 17.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 20 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 22.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 25 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 27.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 30 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 32.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 35 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 37.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 40 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 42.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 45 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 47.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 50 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 52.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 55 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 57.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 60 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 62.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 65 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 67.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 70 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 72.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 75 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 77.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 80 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 82.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 85 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 87.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 90 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 92.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 95 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 97.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 100 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 102.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 105 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 107.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 110 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 112.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 115 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 117.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 120 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 122.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 125 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 127.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 130 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 132.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 135 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 137.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 140 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 142.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 145 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 147.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 150 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 152.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 155 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 157.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 160 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 162.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 165 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 167.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 170 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 172.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 175 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 177.5 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 179 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 180 -sw 0.5 \
	       -next -r 1 -d 1.5 -sA 181 -sw 0.5 \
	-next      -r 1 -d 2.0 -sA 0 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 2.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 7.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 10 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 12.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 15 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 17.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 20 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 22.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 25 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 27.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 30 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 32.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 35 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 37.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 40 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 42.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 45 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 47.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 50 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 52.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 55 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 57.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 60 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 62.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 65 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 67.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 70 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 72.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 75 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 77.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 80 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 82.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 85 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 87.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 90 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 92.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 95 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 97.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 100 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 102.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 105 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 107.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 110 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 112.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 115 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 117.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 120 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 122.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 125 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 127.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 130 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 132.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 135 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 137.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 140 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 142.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 145 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 147.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 150 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 152.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 155 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 157.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 160 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 162.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 165 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 167.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 170 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 172.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 175 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 177.5 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 179 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 180 -sw 0.5 \
	       -next -r 1 -d 2.0 -sA 181 -sw 0.5 \
	-next      -r 1 -d 5.0 -sA 0 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 2.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 7.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 10 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 12.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 15 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 17.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 20 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 22.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 25 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 27.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 30 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 32.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 35 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 37.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 40 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 42.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 45 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 47.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 50 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 52.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 55 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 57.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 60 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 62.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 65 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 67.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 70 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 72.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 75 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 77.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 80 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 82.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 85 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 87.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 90 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 92.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 95 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 97.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 100 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 102.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 105 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 107.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 110 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 112.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 115 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 117.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 120 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 122.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 125 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 127.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 130 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 132.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 135 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 137.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 140 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 142.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 145 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 147.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 150 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 152.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 155 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 157.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 160 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 162.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 165 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 167.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 170 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 172.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 175 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 177.5 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 179 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 180 -sw 0.5 \
	       -next -r 1 -d 5.0 -sA 181 -sw 0.5 \
	-next      -r 1 -d 10.0 -sA 0 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 2.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 7.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 10 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 12.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 15 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 17.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 20 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 22.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 25 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 27.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 30 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 32.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 35 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 37.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 40 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 42.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 45 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 47.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 50 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 52.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 55 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 57.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 60 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 62.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 65 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 67.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 70 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 72.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 75 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 77.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 80 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 82.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 85 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 87.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 90 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 92.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 95 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 97.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 100 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 102.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 105 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 107.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 110 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 112.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 115 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 117.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 120 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 122.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 125 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 127.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 130 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 132.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 135 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 137.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 140 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 142.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 145 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 147.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 150 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 152.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 155 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 157.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 160 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 162.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 165 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 167.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 170 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 172.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 175 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 177.5 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 179 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 180 -sw 0.5 \
	       -next -r 1 -d 10.0 -sA 181 -sw 0.5 \
	-next      -r 1 -d 50.0 -sA 0 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 2.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 7.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 10 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 12.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 15 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 17.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 20 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 22.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 25 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 27.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 30 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 32.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 35 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 37.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 40 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 42.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 45 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 47.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 50 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 52.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 55 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 57.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 60 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 62.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 65 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 67.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 70 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 72.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 75 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 77.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 80 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 82.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 85 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 87.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 90 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 92.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 95 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 97.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 100 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 102.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 105 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 107.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 110 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 112.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 115 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 117.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 120 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 122.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 125 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 127.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 130 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 132.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 135 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 137.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 140 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 142.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 145 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 147.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 150 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 152.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 155 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 157.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 160 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 162.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 165 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 167.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 170 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 172.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 175 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 177.5 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 179 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 180 -sw 0.5 \
	       -next -r 1 -d 50.0 -sA 181 -sw 0.5 \
	       -svg  output_$@.svg -pupil -csv -debug 0
	${ShowInBrowser}  output_$@.svg



cvx_f2: smraytrc
	./smraytrc -convex      -r 1 -d 1.7 -sa 0 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 182.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 185 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 187.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 190 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 192.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 195 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 197.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 200 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 202.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 205 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 207.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 210 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 212.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 215 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 217.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 220 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 222.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 225 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 227.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 230 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 232.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 235 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 237.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 240 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 242.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 245 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 247.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 250 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 252.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 255 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 257.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 260 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 262.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 265 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 267.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 270 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 272.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 275 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 277.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 280 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 282.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 285 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 287.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 290 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 292.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 295 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 297.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 300 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 302.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 305 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 307.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 310 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 312.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 315 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 317.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 320 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 322.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 325 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 327.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 330 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 332.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 335 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 337.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 340 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 342.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 345 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 347.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 350 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 352.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 355 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 357.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 359 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 360 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 361 -sw 0.5 \
	       -svg  output_$@.svg -pupil -csv -debug 0
	${ShowInBrowser}  output_$@.svg

cvx_f3: smraytrc
	./smraytrc -convex \
	             -r 1 -d 1.7 -sa 180 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 182.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 185 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 187.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 190 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 192.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 195 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 197.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 200 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 202.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 205 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 207.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 210 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 212.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 215 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 217.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 220 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 222.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 225 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 227.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 230 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 232.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 235 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 237.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 240 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 242.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 245 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 247.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 250 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 252.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 255 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 257.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 260 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 262.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 265 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 267.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 270 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 272.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 275 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 277.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 280 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 282.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 285 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 287.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 290 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 292.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 295 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 297.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 300 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 302.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 305 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 307.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 310 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 312.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 315 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 317.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 320 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 322.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 325 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 327.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 330 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 332.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 335 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 337.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 340 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 342.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 345 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 347.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 350 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 352.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 355 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 357.5 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 359 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 360 -sw 0.5 \
	       -next -r 1 -d 1.7 -sa 361 -sw 0.5 \
	       -svg  output_$@.svg -pupil -csv -debug 0 -animate
	${ShowInBrowser}  output_$@.svg


cvx_test: smraytrc
	echo Before convex test
	./smraytrc -convex -test
	echo After convext test





ccv_iterate: smraytrc
	./smraytrc -concave  -iterate \
	    	-r 1 \
	       	-sa 0 180 0.5 \
	       	-sw 0.5 \
		-svg  output_$@.svg -pupil
	${ShowInBrowser}  output_$@.svg

ccv_blur: smraytrc
	./smraytrc -concave -r 30 -nr 3 -sw 0.5 \
		-iterate \
	       	-sa 270 360 1 \
		-mw 1 20 1 \
		-svg  output_$@.svg -csv2 sun_a mirror_width ref_blur > output_ccv_blur_blur.csv
	${ShowInBrowser}  output_$@.svg


ccv_width: smraytrc
	./smraytrc -concave -r 30 -nr 20 -sw 0.5 \
		-iterate \
	       	-sa 270 360 1  \
		-mw 1 20 1  \
		-debug \
		-svg  output_$@.svg -csv2 sun_a mirror_width ref_width > output_ccv_width_ref_width.csv
	${ShowInBrowser}  output_$@.svg

ccv_focal: smraytrc
	./smraytrc -concave -r 30 -nr 3 -sw 0.5 \
		-iterate \
	       	-sa 270 360 1 \
		-mw 1 20 1 \
		-svg  output_$@.svg -csv2 sun_a mirror_width ref_focal_p > output_ccv_focal_ref_focal.csv
	${ShowInBrowser}  output_$@.svg

ccv_csv2: smraytrc
	./smraytrc -concave -r 30 -nr 3 -sw 0.5 \
		-iterate \
	       	-sa 0 90 1 \
		-mw 1 20 1 \
		-svg  output_$@.svg -csv2 sun_a mirror_width ref_blur
	${ShowInBrowser}  output_$@.svg

# Names legal with csv2:
#  "radius"
#  "sun_width"
#  "sun_a"
#  "ref_width"
#  "ref_width_p" - percent
#  "ref_focal_d" - distance
#  "ref_focal_p" - percent
#  "ref_blur"
#  "min_normal"
#  "max_normal"
#  "mirror_width"

ccv_zz: smraytrc
	./smraytrc -concave      -r 1  -sa 0 -sw 0.5 \
	       -next -r 1  -sa 2.5 -sw 0.5 \
	       -next -r 1  -sa 5 -sw 0.5 \
	       -next -r 1  -sa 7.5 -sw 0.5 \
	       -next -r 1  -sa 10 -sw 0.5 \
	       -next -r 1  -sa 12.5 -sw 0.5 \
	       -next -r 1  -sa 15 -sw 0.5 \
	       -next -r 1  -sa 17.5 -sw 0.5 \
	       -next -r 1  -sa 20 -sw 0.5 \
	       -next -r 1  -sa 22.5 -sw 0.5 \
	       -next -r 1  -sa 25 -sw 0.5 \
	       -next -r 1  -sa 27.5 -sw 0.5 \
	       -next -r 1  -sa 30 -sw 0.5 \
	       -next -r 1  -sa 32.5 -sw 0.5 \
	       -next -r 1  -sa 35 -sw 0.5 \
	       -next -r 1  -sa 37.5 -sw 0.5 \
	       -next -r 1  -sa 40 -sw 0.5 \
	       -next -r 1  -sa 42.5 -sw 0.5 \
	       -next -r 1  -sa 45 -sw 0.5 \
	       -next -r 1  -sa 47.5 -sw 0.5 \
	       -next -r 1  -sa 50 -sw 0.5 \
	       -next -r 1  -sa 52.5 -sw 0.5 \
	       -next -r 1  -sa 55 -sw 0.5 \
	       -next -r 1  -sa 57.5 -sw 0.5 \
	       -next -r 1  -sa 60 -sw 0.5 \
	       -next -r 1  -sa 62.5 -sw 0.5 \
	       -next -r 1  -sa 65 -sw 0.5 \
	       -next -r 1  -sa 67.5 -sw 0.5 \
	       -next -r 1  -sa 70 -sw 0.5 \
	       -next -r 1  -sa 72.5 -sw 0.5 \
	       -next -r 1  -sa 75 -sw 0.5 \
	       -next -r 1  -sa 77.5 -sw 0.5 \
	       -next -r 1  -sa 80 -sw 0.5 \
	       -next -r 1  -sa 82.5 -sw 0.5 \
	       -next -r 1  -sa 85 -sw 0.5 \
	       -next -r 1  -sa 87.5 -sw 0.5 \
	       -next -r 1  -sa 90 -sw 0.5 \
	       -next -r 1  -sa 92.5 -sw 0.5 \
	       -next -r 1  -sa 95 -sw 0.5 \
	       -next -r 1  -sa 97.5 -sw 0.5 \
	       -next -r 1  -sa 100 -sw 0.5 \
	       -next -r 1  -sa 102.5 -sw 0.5 \
	       -next -r 1  -sa 105 -sw 0.5 \
	       -next -r 1  -sa 107.5 -sw 0.5 \
	       -next -r 1  -sa 110 -sw 0.5 \
	       -next -r 1  -sa 112.5 -sw 0.5 \
	       -next -r 1  -sa 115 -sw 0.5 \
	       -next -r 1  -sa 117.5 -sw 0.5 \
	       -next -r 1  -sa 120 -sw 0.5 \
	       -next -r 1  -sa 122.5 -sw 0.5 \
	       -next -r 1  -sa 125 -sw 0.5 \
	       -next -r 1  -sa 127.5 -sw 0.5 \
	       -next -r 1  -sa 130 -sw 0.5 \
	       -next -r 1  -sa 132.5 -sw 0.5 \
	       -next -r 1  -sa 135 -sw 0.5 \
	       -next -r 1  -sa 137.5 -sw 0.5 \
	       -next -r 1  -sa 140 -sw 0.5 \
	       -next -r 1  -sa 142.5 -sw 0.5 \
	       -next -r 1  -sa 145 -sw 0.5 \
	       -next -r 1  -sa 147.5 -sw 0.5 \
	       -next -r 1  -sa 150 -sw 0.5 \
	       -next -r 1  -sa 152.5 -sw 0.5 \
	       -next -r 1  -sa 155 -sw 0.5 \
	       -next -r 1  -sa 157.5 -sw 0.5 \
	       -next -r 1  -sa 160 -sw 0.5 \
	       -next -r 1  -sa 162.5 -sw 0.5 \
	       -next -r 1  -sa 165 -sw 0.5 \
	       -next -r 1  -sa 167.5 -sw 0.5 \
	       -next -r 1  -sa 170 -sw 0.5 \
	       -next -r 1  -sa 172.5 -sw 0.5 \
	       -next -r 1  -sa 175 -sw 0.5 \
	       -next -r 1  -sa 177.5 -sw 0.5 \
	       -next -r 1  -sa 179 -sw 0.5 \
	       -next -r 1  -sa 180 -sw 0.5 \
	       -next -r 1  -sa 181 -sw 0.5 \
	       -svg  output_$@.svg -pupil -csv -debug 0
	${ShowInBrowser}  output_$@.svg



ccv_svg: smraytrc
	./smraytrc -concave      -r 1  -sa 0 -sw 0.5 \
	       -next -r 1  -sa 2.5 -sw 0.5 \
	       -next -r 1  -sa 5 -sw 0.5 \
	       -next -r 1  -sa 7.5 -sw 0.5 \
	       -next -r 1  -sa 10 -sw 0.5 \
	       -next -r 1  -sa 12.5 -sw 0.5 \
	       -next -r 1  -sa 15 -sw 0.5 \
	       -next -r 1  -sa 17.5 -sw 0.5 \
	       -next -r 1  -sa 20 -sw 0.5 \
	       -next -r 1  -sa 22.5 -sw 0.5 \
	       -next -r 1  -sa 25 -sw 0.5 \
	       -next -r 1  -sa 27.5 -sw 0.5 \
	       -next -r 1  -sa 30 -sw 0.5 \
	       -next -r 1  -sa 32.5 -sw 0.5 \
	       -next -r 1  -sa 35 -sw 0.5 \
	       -next -r 1  -sa 37.5 -sw 0.5 \
	       -next -r 1  -sa 40 -sw 0.5 \
	       -next -r 1  -sa 42.5 -sw 0.5 \
	       -next -r 1  -sa 45 -sw 0.5 \
	       -next -r 1  -sa 47.5 -sw 0.5 \
	       -next -r 1  -sa 50 -sw 0.5 \
	       -next -r 1  -sa 52.5 -sw 0.5 \
	       -next -r 1  -sa 55 -sw 0.5 \
	       -next -r 1  -sa 57.5 -sw 0.5 \
	       -next -r 1  -sa 60 -sw 0.5 \
	       -next -r 1  -sa 62.5 -sw 0.5 \
	       -next -r 1  -sa 65 -sw 0.5 \
	       -next -r 1  -sa 67.5 -sw 0.5 \
	       -next -r 1  -sa 70 -sw 0.5 \
	       -next -r 1  -sa 72.5 -sw 0.5 \
	       -next -r 1  -sa 75 -sw 0.5 \
	       -next -r 1  -sa 77.5 -sw 0.5 \
	       -next -r 1  -sa 80 -sw 0.5 \
	       -next -r 1  -sa 82.5 -sw 0.5 \
	       -next -r 1  -sa 85 -sw 0.5 \
	       -next -r 1  -sa 87.5 -sw 0.5 \
	       -next -r 1  -sa 90 -sw 0.5 \
	       -next -r 1  -sa 92.5 -sw 0.5 \
	       -next -r 1  -sa 95 -sw 0.5 \
	       -next -r 1  -sa 97.5 -sw 0.5 \
	       -next -r 1  -sa 100 -sw 0.5 \
	       -next -r 1  -sa 102.5 -sw 0.5 \
	       -next -r 1  -sa 105 -sw 0.5 \
	       -next -r 1  -sa 107.5 -sw 0.5 \
	       -next -r 1  -sa 110 -sw 0.5 \
	       -next -r 1  -sa 112.5 -sw 0.5 \
	       -next -r 1  -sa 115 -sw 0.5 \
	       -next -r 1  -sa 117.5 -sw 0.5 \
	       -next -r 1  -sa 120 -sw 0.5 \
	       -next -r 1  -sa 122.5 -sw 0.5 \
	       -next -r 1  -sa 125 -sw 0.5 \
	       -next -r 1  -sa 127.5 -sw 0.5 \
	       -next -r 1  -sa 130 -sw 0.5 \
	       -next -r 1  -sa 132.5 -sw 0.5 \
	       -next -r 1  -sa 135 -sw 0.5 \
	       -next -r 1  -sa 137.5 -sw 0.5 \
	       -next -r 1  -sa 140 -sw 0.5 \
	       -next -r 1  -sa 142.5 -sw 0.5 \
	       -next -r 1  -sa 145 -sw 0.5 \
	       -next -r 1  -sa 147.5 -sw 0.5 \
	       -next -r 1  -sa 150 -sw 0.5 \
	       -next -r 1  -sa 152.5 -sw 0.5 \
	       -next -r 1  -sa 155 -sw 0.5 \
	       -next -r 1  -sa 157.5 -sw 0.5 \
	       -next -r 1  -sa 160 -sw 0.5 \
	       -next -r 1  -sa 162.5 -sw 0.5 \
	       -next -r 1  -sa 165 -sw 0.5 \
	       -next -r 1  -sa 167.5 -sw 0.5 \
	       -next -r 1  -sa 170 -sw 0.5 \
	       -next -r 1  -sa 172.5 -sw 0.5 \
	       -next -r 1  -sa 175 -sw 0.5 \
	       -next -r 1  -sa 177.5 -sw 0.5 \
	       -next -r 1  -sa 179 -sw 0.5 \
	       -next -r 1  -sa 180 -sw 0.5 \
	       -next -r 1  -sa 181 -sw 0.5 \
	       -svg  output_$@.svg -pupil -csv -debug 0
	${ShowInBrowser}  output_$@.svg


ccv_animate2: smraytrc
	./smraytrc -concave      -r 30 -mna 280 -mxa 300  -sa 0 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 359 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 358 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 357 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 356 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 355 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 354 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 353 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 352 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 351 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 350 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 349 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 348 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 347 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 346 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 345 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 344 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 343 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 342 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 341 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 340 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 339 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 338 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 337 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 336 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 335 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 334 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 333 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 332 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 331 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 330 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 329 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 328 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 327 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 326 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 325 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 324 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 323 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 322 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 321 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 320 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 319 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 318 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 317 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 316 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 315 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 314 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 313 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 312 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 311 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 310 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 309 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 308 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 307 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 306 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 305 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 304 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 303 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 302 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 301 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 300 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 299 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 298 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 297 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 296 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 295 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 294 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 293 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 292 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 291 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 290 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 289 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 288 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 287 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 286 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 285 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 284 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 283 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 282 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 281 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 280 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 279 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 278 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 277 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 276 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 275 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 274 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 273 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 272 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 271 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 270 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 269 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 268 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 267 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 266 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 265 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 264 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 263 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 262 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 261 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 260 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 259 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 258 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 257 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 256 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 255 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 254 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 253 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 252 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 251 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 250 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 249 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 248 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 247 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 246 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 245 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 244 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 243 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 242 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 241 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 240 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 239 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 238 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 237 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 236 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 235 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 234 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 233 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 232 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 231 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 230 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 229 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 228 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 227 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 226 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 225 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 224 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 223 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 222 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 221 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 220 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 219 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 218 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 217 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 216 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 215 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 214 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 213 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 212 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 211 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 210 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 209 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 208 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 207 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 206 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 205 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 204 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 203 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 202 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 201 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 200 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 199 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 198 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 197 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 196 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 195 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 194 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 193 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 192 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 191 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 190 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 189 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 188 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 187 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 186 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 185 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 184 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 183 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 182 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 181 -sw 0.5 \
	       -next -r 30 -mna 280 -mxa 300  -sa 180 -sw 0.5 \
	       -svg  output_$@.svg -pupil -animate -debug 0
	${ShowInBrowser}  output_$@.svg


ccv_test: smraytrc
	./smraytrc -concave -test
	echo After concave test


ccv_tests: smraytrc
	./smraytrc -concave -r 1 -d 1.1 -sa 0
	./smraytrc -concave -r 1 -d 1.1 -sa 10
	./smraytrc -concave -r 1 -d 1.1 -sa 20
	./smraytrc -concave -r 1 -d 1.1 -sa 30
	./smraytrc -concave -r 1 -d 1.1 -sa 40
	./smraytrc -concave -r 1 -d 1.1 -sa 48.98995
	./smraytrc -concave -r 1 -d 1.1 -sa 48.98996
	./smraytrc -concave -r 1 -d 1.1 -sa 50


#######################################################################
#######################################################################
#######################################################################

example1: example1a example1b example1c example1d example1e

example1a: smraytrc
	./smraytrc -concave -r 10 -sa 290 -mna 180 -mxa 360 -report

example1b: smraytrc
	./smraytrc -concave -r 10 -sa 290 -mna 180 -mxa 360 -report -svg output_$@.svg

example1c: smraytrc
	./smraytrc -concave -nr 6 -r 10 -sa 290 -mna 180 -mxa 360 -report -svg output_$@.svg

example1d: smraytrc
	./smraytrc -concave -nr 6 -focal_pts -r 10 -sa 290 -mna 270 -mxa 330 -report -svg output_$@.svg

example1e: smraytrc
	./smraytrc -concave -nr 5 -focal_pts -r 10 -sa 290 -mna 280 -mxa 300 -svg output_$@.svg


example2: example2a

example2a: smraytrc
	./smraytrc -concave -nr 5 -focal_pts -r 30 -sa 290 -mna 250 -mxa 260 \
			-screen 20 -9.8   10 23 \
			-stencil 10 10  10 -12 \
			-stencil 10 -15  10 -28 \
			-svg output_$@.svg


example3: example3a

example3a: smraytrc
	./smraytrc -concave -nr 5 -focal_pts -r 30 -sa 290 -mna 250 -mxa 260 \
			-screen 20 -9.8   10 23 \
			-stencil 10 10  10 -12 \
			-stencil 10 -15  10 -28 \
			-next -sa 270 \
			-next -sa 310 \
			-svg output_$@.svg



example4: example4a
example4a: smraytrc
	./smraytrc -concave -nr 3 -focal_pts -r 30 -mna 250 -mxa 260 \
			-screen 20 -9.8   10 23 \
			-stencil 10 10  10 -12 \
			-stencil 10 -15  10 -28 \
			-iterate -sa 255 315 5 \
			-svg output_$@.svg

example5: example5a
example5a: smraytrc
	./smraytrc -concave -nr 5 -focal_pts -r 30 -mna 250 -mxa 260 \
			-screen 20 -9.8   10 23 \
			-stencil 10 10  10 -12 \
			-stencil 10 -15  10 -28 \
			-iterate -sa 255 315 1 \
			-animate \
			-svg output_$@.svg



example6: example6a example6b
example6a: smraytrc
	./smraytrc -concave -r 30 -mna 225 -mxa 260 -sa 260 \
			-screen 20 -9.8   10 23 \
			-stencil 10 10  10 -12 \
			-stencil 10 -15  10 -28 \
			-reverse \
			-svg output_$@.svg

example6b: smraytrc
	./smraytrc -concave -r 30 -mna 225 -mxa 260 -sa 260 \
			-screen 20 -9.8   10 23 \
			-stencil 10 10  10 -12 \
			-stencil 10 -15  10 -28 \
			-reverse \
			-target 15 6.6 \
			-target 10 -12.6 \
			-target 10 -14 \
			-svg output_$@.svg \
			-report

example7: example7a example7b example7c example7d

example7a: smraytrc
	./smraytrc -convex -r 30 -mna 10 -mxa 140 -sa 220 \
	-d 30.01 \
	-pupil \
			-screen 20 -9.8   10 23 \
			-stencil 10 10  10 -12 \
			-stencil 10 -15  10 -28 \
			-svg output_$@.svg \

example7b: smraytrc
	./smraytrc -convex -r 30 -mna 10 -mxa 140 -sa 220 \
	-d 50 \
	-pupil \
			-screen 20 -9.8   10 23 \
			-stencil 10 10  10 -12 \
			-stencil 10 -15  10 -28 \
			-svg output_$@.svg \

example7c: smraytrc
	./smraytrc -convex -r 30 -mna 10 -mxa 140 -sa 220 \
	-d 100 \
	-pupil \
			-screen 20 -9.8   10 23 \
			-stencil 10 10  10 -12 \
			-stencil 10 -15  10 -28 \
			-svg output_$@.svg \

example7d: smraytrc
	./smraytrc -convex -r 30 -mna 10 -mxa 140 -sa 220 \
	-d 1000 \
	-pupil \
			-screen 20 -9.8   10 23 \
			-stencil 10 10  10 -12 \
			-stencil 10 -15  10 -28 \
			-svg output_$@.svg \


