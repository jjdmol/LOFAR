all: Main.class

include jarpath.include

Main.class:
	javac -deprecation src/org/astron/lofarsim/*.java \
		-classpath $(SAXJAR):$(KIWIJAR) \
		-sourcepath src \
		-d classes
	javac -deprecation src/org/astron/basesim/Main.java \
		-classpath $(SAXJAR):$(KIWIJAR) \
		-sourcepath src \
		-d classes
