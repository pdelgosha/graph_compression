#include "marked_graph_compression.h"

void marked_graph_compressed::clear()
{
  star_edges.clear();
  message_list.clear();
  ver_type_list.clear();
  part_bgraph.clear();
  part_graph.clear();
}




marked_graph_compressed marked_graph_encoder::encode(const marked_graph& G)
{
  compressed.clear(); // reset the compressed variable before starting

  n = G.nu_vertices;
  compressed.n = n;

  extract_edge_types(G);
  compressed.L = L;
  compressed.ver_type_list = C.ver_type_list; // 
  compressed.message_list = vector<vector<int> >(C.M.message_list.begin(), C.M.message_list.begin() + L); // taking the first L elements corresponding to non star type edges in the message list of M 

  cout << " message list " << endl;
  for (int i=0;i<C.M.message_list.size();i++){
    cout << i << " : ";
    for (int j=0;j<C.M.message_list[i].size();j++)
      cout << C.M.message_list[i][j] << " ";
    cout << endl;
  }

  encode_star_vertices(); // encode the list of vertices with at least one star edge connected to them


  encode_star_edges(); // encode edges with star types, i.e. those with half edge type L or larger


  encode_vertex_types(); // encode the sequences \f$\vec{\beta}, \vec{D}\f$, which is encoded in C.ver_type


  extract_partition_graphs(); // for equality types, we form simple unmarked graphs, and for inequality types, we form a bipartite graph

  cout << " partition bipartite graphs " << endl;
  for (map<pair<int, int>, b_graph>::iterator it = part_bgraph.begin(); it!=part_bgraph.end(); it++){
    cout << " c = " << it->first.first << " , " << it->first.second << endl;
    cout << it->second << endl;
  }

  cout << " partition simple graphs " << endl;
  for (map<int, graph>::iterator it = part_graph.begin();it!=part_graph.end();it++){
    cout << " t = " << it->first << endl;
    cout << it->second << endl;
  }

  encode_partition_bgraphs();


  encode_partition_graphs();


  return compressed;
}

void marked_graph_encoder::encode_vertex_types()
{
  time_series_encoder vtype_encoder(n);
  //cerr << " C.ver_type_int " << endl;
  //for (int i=0;i<C.ver_type_int.size();i++)
  //  cerr << C.ver_type_int[i] << " ";
  //cerr << endl;
  compressed.ver_types = vtype_encoder.encode(C.ver_type_int);
}

void marked_graph_encoder::extract_edge_types(const marked_graph& G)
{
  // extracting edges types (aka colors)
  C = colored_graph(G, h, delta);
  is_star_vertex = C.is_star_vertex;
  star_vertices = C.star_vertices;
  L = C.L; // the number of colors, excluding star edges 
}

void marked_graph_encoder::encode_star_vertices()
{
  // compress the is_star_vertex list
  time_series_encoder star_encoder(n);
  compressed.star_vertices = star_encoder.encode(is_star_vertex);
}

void marked_graph_encoder::encode_star_edges()
{
  int x, xp; // auxiliary mark variables
  int w; // auxiliary vertex variable
  int v; // auxiliary vertex variable 
  for (int k=0; k<star_vertices.size(); k++){ // iterating over star vertices
    v = star_vertices[k];
    for (int i=0;i<C.adj_list[v].size();i++){
      if (C.adj_list[v][i].second.first > L or C.adj_list[v][i].second.second > L){ // this is a star edge
        x = C.M.message_list[C.adj_list[v][i].second.first][1]; // mark towards v
        xp = C.M.message_list[C.adj_list[v][i].second.second][1]; // mark towards other endpoint
        w = C.adj_list[v][i].first; // the other endpoint of the edge
        if (x < xp){ // if x > xp, we only store this edge when visiting the other endpoint (w), since we do not want to express an edge twice
          if (compressed.star_edges.find(pair<int, int>(x,xp)) == compressed.star_edges.end()) // this pair does not exist
            compressed.star_edges[pair<int, int>(x,xp)].resize(star_vertices.size()); // open space for all star vertices 
          compressed.star_edges[pair<int, int>(x,xp)][k].push_back(w); // add w to the position of v (which is k)
        }
        if (x == xp and w > v){ // if w < v, we store this edge when visiting the other endpoint (w) to avoid storing and edge twice
          if (compressed.star_edges.find(pair<int, int>(x,xp)) == compressed.star_edges.end()) // not yet exist
            compressed.star_edges[pair<int, int>(x,xp)].resize(star_vertices.size()); // open space
          compressed.star_edges[pair<int, int>(x,xp)][k].push_back(w);
        }
      }
    }
  }
}

void marked_graph_encoder::extract_partition_graphs()
{
  map< pair<int, int> , vector<vector<int> > > part_adj_list; // for \f$0 \leq t \leq t' < L\f$, part_adj_list[pair<int, int>(t,t')] is an adjacency list. If \f$t < t'\f$, this is the adjacency list of the partition bipartite graph corresponding to the edges with color \f$(t,t')\f$. If \f$t = t'\f$, this corresponds to the partition simple graph corresponding to edges with color \f$(t,t')\f$.

  int t, tp;
  int w;
  for (int v=0; v<n; v++){
    for (int i=0;i<C.adj_list[v].size();i++){
      w = C.adj_list[v][i].first; // other endpoint
      t = C.adj_list[v][i].second.first; // half edge type towards v
      tp = C.adj_list[v][i].second.second; // half edge type towards the other endpoint
      if (t < L and tp < L){ // this is a normal edge, i.e. not a star edge
        if (t < tp){ // this edge has a color in \f$C_<\f$
          if (part_adj_list.find(pair<int, int>(t,tp)) == part_adj_list.end()){
            // the observed color does not yet exist, we need to first create it
            part_adj_list[pair<int, int>(t,tp)].resize(n);
          }
          part_adj_list[pair<int, int>(t,tp)][v].push_back(w); // add w as a neighbor
        }
        if (t == tp){ // this edge has a color in \f$C_=\f$
          if (v < w){ // if v > w, v is already recorded as a neighbor of w, since we need to keep track of the forwards adjacency list
            if (part_adj_list.find(pair<int, int>(t,tp)) == part_adj_list.end()){
              // first, create the forward adjacency list
              part_adj_list[pair<int, int>(t,tp)].resize(n);
            }
            part_adj_list[pair<int, int>(t, tp)][v].push_back(w);
          }
        }
      }
    }
  }

  //cerr << " extracted adjacency lists " << endl;

  // going over part_adj_list and construct partition graphs and bipartite graphs
  pair<int, int> c; // the color
  vector<vector<int> > list; // the adjacency list / forward adjacency list
  b_graph test;
  //cerr << " part_adj_list.size() " << part_adj_list.size() << endl;
  for (map<pair<int, int>, vector<vector<int> > >::iterator it = part_adj_list.begin(); it!=part_adj_list.end(); it++){
    c = it->first;
    list = it->second;
    if (c.first < c.second) // this is a color in \f$C_<\f$
      part_bgraph[c] = b_graph(list);

    if (c.first == c.second) // this is a color in \f$C_=\f$
      part_graph[c.first] = graph(list);

  }
}


void marked_graph_encoder::encode_partition_bgraphs()
{
  vector<int> a, b; // degree sequences to initialize graph encoders 
  a.resize(n);
  b.resize(n);
  int t, tp;

  // compressing bipartite graphs 
  for (map<pair<int, int> , b_graph>::iterator it = part_bgraph.begin(); it!=part_bgraph.end(); it++){
    // the color components are t, tp
    t = it->first.first; // type towards me (left vertices)
    tp = it->first.second; // type towards other endpoints (right vertices)
    // finding a and b
    //for (int v=0;v<n;v++){
    //  a[v] = C.ver_type[v][1+ t*L + tp]; // C.ver_type[v] was a vector of size 1 + L x L, where the fist entry is the vertex mark, and the rest is the degree matrix, so we can read degree sequences of the bipartite graph from this array
    //  b[v] = C.ver_type[v][1+ tp*L + t];
    // }
    a = it->second.get_left_degree_sequence();
    b = it->second.get_right_degree_sequence();

    b_graph_encoder E(a,b);

    compressed.part_bgraph[pair<int, int>(t,tp)] = E.encode(it->second);
  }

}

void marked_graph_encoder::encode_partition_graphs()
{
  vector<int> a; // degree sequence to initialize graph encoders 
  a.resize(n);
  int t;

  // compressing graphs
  for (map<int, graph>::iterator it=part_graph.begin(); it!=part_graph.end(); it++){
    t = it->first; // the color is t,t
    a = it->second.get_degree_sequence();

    graph_encoder E(a);

    compressed.part_graph[t] = E.encode(it->second);
  }  
}
