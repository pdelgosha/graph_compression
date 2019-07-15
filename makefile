all:
	doxygen Doxyfile
	g++ -std=c++11  -I /opt/local/include -O3 -static /opt/local/lib/libgmp.a gcomp.cpp  marked_graph.cpp graph_message.cpp fenwick.cpp simple_graph.cpp simple_graph_compression.cpp compression_helper.cpp bipartite_graph.cpp bipartite_graph_compression.cpp time_series_compression.cpp marked_graph_compression.cpp random_graph.cpp logger.cpp -lgmpxx -lgmp  -o gcomp_2.o

rnd_graph:
	g++ -std=c++11  -I /opt/local/include -o2 rnd_graph.cpp marked_graph.cpp graph_message.cpp fenwick.cpp simple_graph.cpp simple_graph_compression.cpp compression_helper.cpp bipartite_graph.cpp bipartite_graph_compression.cpp time_series_compression.cpp marked_graph_compression.cpp random_graph.cpp logger.cpp -lgmpxx -lgmp  -o rnd_graph.o

testmp:
	g++ -std=c++11 -I /opt/local/include -o2 test_mp.cpp marked_graph.cpp graph_message.cpp  random_graph.cpp logger.cpp -lgmpxx -lgmp  -o test_mp.o
	time ./test_mp.o

testcp:
	g++ -std=c++11  -I /opt/local/include -O3 test.cpp  marked_graph.cpp graph_message.cpp fenwick.cpp simple_graph.cpp simple_graph_compression.cpp compression_helper.cpp bipartite_graph.cpp bipartite_graph_compression.cpp time_series_compression.cpp marked_graph_compression.cpp random_graph.cpp logger.cpp -lgmpxx -lgmp  -o test.o
	./test.o
