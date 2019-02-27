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
  compressed.h = h;
  compressed.delta = delta;

  extract_edge_types(G);
  cout << " edge types extracted " << endl;

  compressed.L = L;
  compressed.ver_type_list = C.ver_type_list; // 
  compressed.message_list = vector<vector<int> >(C.M.message_list.begin(), C.M.message_list.begin() + L); // taking the first L elements corresponding to non star type edges in the message list of M 

  /*
  cout << " message list " << endl;
  for (int i=0;i<C.M.message_list.size();i++){
    cout << i << " : ";
    for (int j=0;j<C.M.message_list[i].size();j++)
      cout << C.M.message_list[i][j] << " ";
    cout << endl;
  }
  */

  encode_star_vertices(); // encode the list of vertices with at least one star edge connected to them
  cout << " encoded star vertices " << endl;

  encode_star_edges(); // encode edges with star types, i.e. those with half edge type L or larger
  cout << " encoded star edges " << endl;

  encode_vertex_types(); // encode the sequences \f$\vec{\beta}, \vec{D}\f$, which is encoded in C.ver_type
  cout << " encoded vertex types " << endl;

  extract_partition_graphs(); // for equality types, we form simple unmarked graphs, and for inequality types, we form a bipartite graph
  cout << " extracted partition graphs " << endl;
  /*
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
  */

  encode_partition_bgraphs();
  cout << " encoded partition bgraphs " << endl;

  encode_partition_graphs();
  cout << " encoded partition graphs " << endl;

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
      if (C.adj_list[v][i].second.first >= L or C.adj_list[v][i].second.second >= L){ // this is a star edge
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









// ==================================================
// marked graph decoder
// ==================================================




marked_graph marked_graph_decoder::decode(const marked_graph_compressed& compressed)
{
  n = compressed.n;
  h = compressed.h;
  delta = compressed.delta;
  L = compressed.L;

  edges.clear(); // clear the edge list of the marked graph to be decoded
  vertex_marks.clear(); // clear the list of vertex marks of the marked graph to be decoded

  decode_star_vertices(compressed);
  cerr << " decoded star vertices " << endl;

  decode_star_edges(compressed);
  cerr << " decoded star edges " << endl;

  decode_vertex_types(compressed);
  cerr << " decoded vertex types " << endl;

  decode_partition_graphs(compressed);
  cerr << " decoded partition graphs " << endl;

  decode_partition_bgraphs(compressed);
  cerr << " decoded partition b graphs " << endl;

  // now, reconstruct the original marked graphs by assembling the vertex marks and edge list
  marked_graph G(n, edges, vertex_marks);
  return G;
}

void marked_graph_decoder::decode_star_vertices(const marked_graph_compressed& compressed)
{
  time_series_decoder D(n);
  is_star_vertex = D.decode(compressed.star_vertices);

  star_vertices.clear();
  for (int i=0;i<n;i++)
    if (is_star_vertex[i] == 1)
      star_vertices.push_back(i);
}

void marked_graph_decoder::decode_star_edges(const marked_graph_compressed& compressed)
{
  pair<int, int> mark_pair; // the pair of marks
  vector<vector<int> > list; // list of edges with this pair of marks
  int v; // one endpoint of the star edge
  // iterating through the star_edges map
  for (map<pair<int, int>, vector<vector<int> > >::const_iterator it = compressed.star_edges.begin(); it!=compressed.star_edges.end(); it++){
    mark_pair = it->first;
    //cerr << " mark_pair " << mark_pair.first << " " << mark_pair.second << endl;
    list = it->second;
    for (int i=0;i<list.size();i++){
      v = star_vertices[i];
      for (int j=0;j<list[i].size();j++){
        //cerr << " list[i][j] " << list[i][j] << endl;
        edges.push_back(pair<pair<int, int>, pair<int, int> >(pair<int, int>(v,list[i][j]), mark_pair));
      }
    }
  }
}

void marked_graph_decoder::decode_vertex_types(const marked_graph_compressed& compressed)
{
  time_series_decoder D(n);
  vector<int> ver_type_int = D.decode(compressed.ver_types);

  // converting the integer value vertex types to actual vectors using the `ver_type_list` attribute of compressed

  vertex_marks.resize(n); // preparing for decoding vertex marks
  ver_type.resize(n);

  for (int v=0;v<n;v++){
    if (ver_type_int[v] >= compressed.ver_type_list.size())
      cerr << " Warning: marked_graph_decoder::decode_vertex_types ver_type_int[" << v << "] is out of range" << endl;
    ver_type[v] = compressed.ver_type_list[ver_type_int[v]];
    vertex_marks[v] = ver_type[v][0]; // the mark of vertex v is the first element in the type list of this vertex 
  }
}


void marked_graph_decoder::decode_partition_graphs(const marked_graph_compressed& compressed)
{
  int t; // the type corresponding to the partition graph
  vector<int> t_message; // the actual message corresponding to t
  int x; // the mark component associated to t
  pair< mpz_class, vector< int > > G_compressed; // the compressed form of the partition graph
  graph G; // the decoded partition graph
  vector<int> a; // the degree sequence of the partition graph to be decoded
  a.resize(n);
  vector<int> flist; // the forward adjacency list of a vertex in a partition graph

  for(map< int, pair< mpz_class, vector< int > > >::const_iterator it=compressed.part_graph.begin(); it!=compressed.part_graph.end(); it++){
    t = it->first;
    t_message = compressed.message_list[t];
    if (t_message.size() == 0)
      cerr << " Warning: marked_graph_decoder::decode_partition_graphs t_message corresponding to t = " << t << " has size 0" << endl;
    x = t_message[t_message.size()-1]; // the mark component of a message is always the last entry in the list 
    G_compressed = it->second;
    // trying to reconstruct the partition graph, but before that, we need to figure out its degree sequence based on the `ver_type` sequence and reading the corresponding entry of the degree matrix of each vertex
    for (int v=0;v<n;v++){
      a[v] = ver_type[v][1+ L*t + t]; // the degree is the (t,t) coordinate of the degree matrix. Since the first element of `ver_type[v]` is the vertex mark and we list the degree matrix row by row, this is precisely the index 1 + L*t + t in the list.
    }
    graph_decoder D(a);
    G = D.decode(G_compressed.first, G_compressed.second);
    // for each edge in G, we should add an edge with mark pair (x,x) to the edge list of the marked graph
    for (int v=0;v<n;v++){
      flist = G.get_forward_list(v);
      for (int i=0;i<flist.size();i++)
        edges.push_back(pair<pair<int, int>, pair<int, int> >(pair<int, int>(v,flist[i]), pair<int, int> (x,x)));
    }
  }
}


void marked_graph_decoder::decode_partition_bgraphs(const marked_graph_compressed& compressed)
{
  pair<int, int> c; // the pair of types
  int t, tp; // types
  vector<int> t_message, tp_message; // associated messages to types
  int x, xp; // mark components of t and tp

  b_graph G;
  vector<int> a, b; // the left and right degree sequences
  a.resize(n);
  b.resize(n);
  vector<int> adj_list; // adj list of a vertex in a partition bipartite graph

  for (map<pair<int, int>, mpz_class>::const_iterator it = compressed.part_bgraph.begin(); it!=compressed.part_bgraph.end(); it++){
    c = it->first;
    t = c.first;
    tp = c.second;
    t_message = compressed.message_list[t];
    tp_message = compressed.message_list[tp];
    if (t_message.size() == 0)
      cerr << " Warning: marked_graph_decoder::decode_partition_bgraphs t_message corresponding to t = " << t << " has size 0" << endl;
    if (tp_message.size() == 0)
      cerr << " Warning: marked_graph_decoder::decode_partition_bgraphs tp_message corresponding to tp = " << tp << " has size 0" << endl;
    x = t_message[t_message.size()-1];
    xp = tp_message[tp_message.size()-1];
    for (int v=0;v<n;v++){
      a[v] = ver_type[v][1+t*L + tp]; // row t column tp of degree matrix, +1 since the first entry is the vertex mark 
      b[v] = ver_type[v][1+tp*L + t]; // rot tp column t
    }
    b_graph_decoder D(a,b);
    G = D.decode(it->second);
    for (int v=0;v<n;v++){
      adj_list = G.get_adj_list(v);
      for (int i=0;i<adj_list.size();i++)
        edges.push_back(pair<pair<int, int>, pair<int, int> >(pair<int, int>(v,adj_list[i]), pair<int, int>(x,xp)));
    }
  }
}
