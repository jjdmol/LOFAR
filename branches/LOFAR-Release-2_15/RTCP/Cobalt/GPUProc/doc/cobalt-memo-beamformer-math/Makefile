NAME=cobalt-memo-beamformer-math

.PHONY: $(NAME).pdf

$(NAME).pdf: $(NAME).tex Makefile
	rm *.aux *.toc *.idx ; pdflatex $< && bibtex  $(NAME) && makeindex $(NAME) && pdflatex $< && pdflatex $< 


