all:
	g++ Differentiator.cpp logs.cpp main.cpp MyGeneralFunctions.cpp Syntax_analyzer.cpp Tree.cpp advanced_stack.cpp -o Diff.out

debug: 
	g++ Differentiator.cpp logs.cpp main.cpp MyGeneralFunctions.cpp Syntax_analyzer.cpp Tree.cpp advanced_stack.cpp -o Diff.out -g