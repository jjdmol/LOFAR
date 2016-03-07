NAME=cobalt-commissioning-report

.PHONY: $(NAME).pdf

$(NAME).pdf: $(NAME).tex
	rm *.aux *.toc; pdflatex $< && bibtex  $(NAME) && pdflatex $< && pdflatex $< 
