DOCUMENT := document
LATEXMK := latexmk -pdf
BIBTEX := bibtex
MAKEGLOSSARIES := makeglossaries

.PHONY: all document clean

all: document bibtex glossaries document document

document:
	@echo "Build document"
	$(LATEXMK) $(DOCUMENT).tex

bibtex:
	@echo "Build bibtex"
	$(BIBTEX) $(DOCUMENT)

glossaries:
	@echo "Build glossaries"
	$(MAKEGLOSSARIES) $(DOCUMENT)

clean:
	@ echo 'Clean up'
	$(LATEXMK) -C
	rm -f $(DOCUMENT).{acn,acr,alg,bbl,glg,glo,gls,ist,lol,run.xml}
	rm -f $(DOCUMENT)-{blx.bib,blx.aux}

