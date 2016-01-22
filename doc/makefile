all:fig
	xelatex main.tex
	bibtex main
	xelatex main.tex
	xelatex main.tex
	evince main.pdf &
	rm -rf *.aux *.idx *.log *.toc *.out *.bbl *.blg *.lot *.lof
       
fig:
	make -C fig

clean:
	make -C fig clean
	rm -rf *.aux *.idx *.log *.toc *.out *.bbl *.blg *.lot *.lof *.pdf

.PHONY : all fig clean

