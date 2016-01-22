all:
	xelatex main.tex
	#bibtex main
	xelatex main.tex
	xelatex main.tex
	rm -rf *.aux *.idx *.log *.toc *.out *.bbl *.blg *.lot *.lof
	mv main.pdf 申报文档.pdf
	evince 申报文档.pdf &
       
fig:
	make -C fig

clean:
	#make -C fig clean
	rm -rf *.aux *.idx *.log *.toc *.out *.bbl *.blg *.lot *.lof *.pdf

.PHONY : all fig clean

