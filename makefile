test:
	doxygen Doxyfile
	g++ -std=c++11 -o2 test.cpp marked_graph.cpp graph_message.cpp fenwick.cpp simple_graph.cpp simple_graph_compression.cpp compression_helper.cpp bipartite_graph.cpp bipartite_graph_compression.cpp time_series_compression.cpp marked_graph_compression.cpp random_graph.cpp -lgmpxx -lgmp  -o test.o
	time ./test.o
