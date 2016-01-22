all: 3dRotateByZ.pdf

3dRotateByZ.pdf:3dRotateByZ.asy
	asy -render=0 *.asy

draw:all
	evince *.pdf

clean:
	rm -rf *.aux *.log
	rm -rf 6axis_x.pdf 2dRotate.pdf 3dRotateByZ.pdf 9axis.pdf 6axis_y.pdf eastor.pdf 

.PHONY : clean

