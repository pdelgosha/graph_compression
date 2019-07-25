#include "marked_graph_compression.h"

void marked_graph_compressed::clear()
{
  star_edges.clear();
  type_mark.clear();
  ver_type_list.clear();
  part_bgraph.clear();
  part_graph.clear();
}

/*!
  \param f: a `FILE*` object which is the address of the binary file to write
 */
void marked_graph_compressed::binary_write(FILE* f){

  logger::current_depth++;
  // ==== write n, h, delta
  logger::add_entry("n", "");
  fwrite(&n, sizeof n, 1, f);
  logger::add_entry("h", "");
  fwrite(&h, sizeof h, 1, f);
  logger::add_entry("delta", "");
  fwrite(&delta, sizeof delta, 1, f);

  logger::add_entry("type_mark", "");
  int int_out; // auxiliary variable, an integer value to be written to output
  // ==== write type_mark
  // first, the number of types
  int_out = type_mark.size();
  fwrite(&int_out, sizeof int_out, 1, f);
  // then, marks one by one
  for (int i=0;i<type_mark.size();i++){
    int_out = type_mark[i];
    fwrite(&int_out, sizeof int_out, 1, f);
  }

  logger::add_entry("star_vertices", "");
  // ==== write star vertices
  // first, write the frequency, note that star_vertices.first is a vector of size 2 with the first entry being the number of zeros, and the second one the number of ones, so it enough to write only one of them
  int_out = star_vertices.first[0];
  fwrite(&int_out, sizeof int_out, 1, f);

  // then, we write the integer representation star_vertices.second
  mpz_out_raw(f, star_vertices.second.get_mpz_t());

  logger::add_entry("star_edges", "");
  // ==== write star edges

  int log2n = 0; // the ceiling of log (n+1) in base 2 (which is equal to 1 + the floor of log_2 n), which is the number of bits to encode vertices
  int n_copy = n;
  while(n_copy > 0){
    n_copy >>= 1;
    log2n ++;
  }
  //cerr << " log2n " << log2n << endl;
  bitset<8*sizeof(int)> B; // a bit stream with maximum length of int to store a vertex index

  map<pair<int, int>, vector<vector<int> > >::iterator it;
  int x, xp;
  string s; // the bit stream

  // first, write the size of star_edges so that the decoder knows how many blocks are coming
  int_out = star_edges.size();
  fwrite(&int_out, sizeof int_out, 1, f);

  int nu_star_edges = 0; // number of star edges 
  for (it = star_edges.begin(); it!= star_edges.end(); it++){
    x = it->first.first;
    xp = it->first.second;
    //write x and xp
    fwrite(&x, sizeof x, 1, f);
    fwrite(&xp, sizeof xp, 1, f);
    s = "";
    for (int i=0;i<it->second.size();i++){
      for(int j=0;j<it->second[i].size();j++){
        s += "1";
        B = it->second[i][j]; // convert the index of the other endpoint to binary
        s += B.to_string().substr(8*sizeof(int) - log2n, log2n); // take only log2n many bits of the representation (and this should be taken from the least significant bits)
        nu_star_edges ++;
      }
      s += "0"; // to indicate that the neighbor list of this vertex is over now
    }
    //cerr << " write  x " << x << " xp " << xp << " s " << s << endl;
    //for (int i=0;i<it->second.size();i++){
    //  for (int j=0;j<it->second[i].size();j++){
    //    cerr << " , " << it->second[i][j];
    //  }
    //  cerr << endl;
    //}
    bit_string_write(f, s); // write this bitstream to the output
  }

  logger::add_entry("vertex types", "");

  // ==== write vertex types

  // first, we need vertex types list (ver_type_list)
  // size of ver_type_list
  int_out = ver_type_list.size();
  fwrite(&int_out, sizeof int_out, 1, f);
  for (int i=0;i<ver_type_list.size();i++){
    int_out = ver_type_list[i].size();
    fwrite(&int_out, sizeof int_out, 1, f);
    for (int j=0;j<ver_type_list[i].size();j++){
      int_out = ver_type_list[i][j];
      fwrite(&int_out, sizeof int_out, 1, f);
    }
  }

  // then, write ver_types

  // ver_types.first
  // ver_types.first.size():
  int_out = ver_types.first.size();
  fwrite(&int_out, sizeof int_out, 1, f);
  for (int i =0;i<ver_types.first.size(); i++){
    int_out = ver_types.first[i];
    fwrite(&int_out, sizeof int_out, 1, f);
  }
  // ver_types.second
  mpz_out_raw(f, ver_types.second.get_mpz_t());


  logger::add_entry("partition bipartite graphs", "");
  // ==== part bgraphs

  // part_bgraphs.size
  int_out = part_bgraph.size();
  fwrite(&int_out, sizeof int_out, 1, f);
  map<pair<int, int>, mpz_class>::iterator it2;
  if (logger::stat){
    *logger::stat_stream << " ==== statistics ==== " << endl;
    *logger::stat_stream << " n:                " << n << endl;
    *logger::stat_stream << " h:                " << h << endl;
    *logger::stat_stream << " delta:            " << delta << endl;
    *logger::stat_stream << " No. types         " << type_mark.size() << endl;
    *logger::stat_stream << " No. * vertices    " << n - star_vertices.first[0] << endl;
    *logger::stat_stream << " No. * edges       " << nu_star_edges << endl;
    *logger::stat_stream << " No. part bgraphs  " << part_bgraph.size() << endl;
    *logger::stat_stream << " No. part graphs   " << part_graph.size() << endl;
  }

  for (it2 = part_bgraph.begin(); it2 != part_bgraph.end(); it2++){
    // first, write t, t'
    int_out = it2->first.first;
    fwrite(&int_out, sizeof int_out, 1, f);
    int_out = it2->first.second;
    fwrite(&int_out, sizeof int_out, 1, f);
    // then, the compressed integer
    mpz_out_raw(f, it2->second.get_mpz_t());
  }

  logger::add_entry("partition graphs", "");
  // === part graphs

  // part_graph.size
  int_out = part_graph.size();
  fwrite(&int_out, sizeof int_out, 1, f);
  map< int, pair< mpz_class, vector< int > > >::iterator it3;
  for (it3 = part_graph.begin(); it3 != part_graph.end(); it3++){
    int_out = it3->first; // the type
    fwrite(&int_out, sizeof int_out, 1, f);
    // the mpz part
    mpz_out_raw(f, it3->second.first.get_mpz_t());
    // the vector part
    // first its size
    int_out = it3->second.second.size();
    fwrite(&int_out, sizeof int_out, 1, f);
    // then element by element
    for(int j=0;j<it3->second.second.size();j++){
      int_out = it3->second.second[j];
      fwrite(&int_out, sizeof int_out, 1, f);
    }
  }

  logger::current_depth--;
}

/*!
  \param f: a `FILE*` object which is the address of the binary file to write
*/
void marked_graph_compressed::binary_read(FILE* f){
  clear(); // to make sure nothing is stored inside me before reading

  // ==== read n, h, delta
  fread(&n, sizeof n, 1, f);
  fread(&h, sizeof h, 1, f);
  fread(&delta, sizeof delta, 1, f);

  int int_in; // auxiliary input integer
  // ===== read type_mark
  // read number of types
  fread(&int_in, sizeof int_in, 1, f);
  type_mark.resize(int_in);
  for (int i=0;i<type_mark.size();i++){
    fread(&int_in, sizeof int_in, 1, f);
    type_mark[i] = int_in;
  }

  // ==== read star_vertices
  // first, read the frequency.
  star_vertices.first = vector<int>(2); // frequency,
  // we read its first index which is number of zeros, and the second is n - the first.
  fread(&int_in, sizeof int_in, 1, f);
  star_vertices.first[0] = int_in;
  star_vertices.first[1] = n - int_in;

  // the integer representation which is star_vertices.second
  mpz_inp_raw(star_vertices.second.get_mpz_t(), f);

  // ==== read star_edges

  int log2n = 0; // the ceiling of log (n+1) in base 2 (which is equal to 1 + the floor of log_2 n), which is the number of bits to encode vertices
  int n_copy = n;
  while(n_copy > 0){
    n_copy >>= 1;
    log2n ++;
  }
  bitset<8*sizeof(int)> B; // a bit stream with maximum length of int to store a vertex index

  string s;
  stringstream ss;
  int sp; // the index of the string s we are studying 

  // read the size of star_edges

  int star_edges_size;
  fread(&star_edges_size, sizeof star_edges_size, 1, f);

  int x, xp; // edge marks
  int nu_star_vertices = star_vertices.first[1];

  vector<vector<int> > V; // the list of star edges corresponding to each mark pair
  V.resize(nu_star_vertices);

  for (int i=0;i<star_edges_size;i++){
    fread(&x, sizeof x, 1, f);
    fread(&xp, sizeof xp, 1, f);
  
    s = bit_string_read(f);
    //cerr << " read  x " << x << " xp " << xp << " s " << s << endl;
    sp = 0; // starting from zero 
    for (int j=0; j<nu_star_vertices; j++){ // 
      V[j].clear(); // make it fresh
      while(s[sp++] == '1'){ // there is still some edge connected to this vertex 
        // read log2n many bits
        //cerr << " s subtr " << s.substr(sp, log2n);
        //ss << s.substr(sp, log2n);
        B = bitset<8*sizeof(int)>(s.substr(sp, log2n));
        //cerr << " ss " << ss.str() << endl;
        sp += log2n;
        //ss >> B;

        V[j].push_back(B.to_ulong());
      }
      //for (int k=0;k<V[j].size();k++)
      //  cerr << " , " << V[j][k];
      //cerr << endl;
    }


    star_edges.insert(pair< pair<int, int> , vector<vector<int> > > (pair<int, int>(x, xp), V));
  }

  // ==== read vertex_types

  // read ver_type_list
  fread(&int_in, sizeof int_in, 1, f); // size of ver_type_list
  ver_type_list.resize(int_in);
  for (int i=0; i<ver_type_list.size();i++){
    fread(&int_in, sizeof int_in, 1, f); // size of ver_type_list[i]
    ver_type_list[i].resize(int_in);
    for (int j=0;j<ver_type_list[i].size();j++){
      fread(&int_in, sizeof int_in, 1, f);
      ver_type_list[i][j] = int_in;
    }
  }

  // ver_types
  // ver_types.first
  // ver_types.first.size()
  fread(&int_in, sizeof int_in, 1, f);
  ver_types.first.resize(int_in);
  for (int i=0;i<ver_types.first.size();i++){
    fread(&int_in, sizeof int_in, 1, f);
    ver_types.first[i] = int_in;
  }
  // ver_types.second
  mpz_inp_raw(ver_types.second.get_mpz_t(), f);


  // === part bgraphs
  int part_bgraph_size;
  int t, tp;
  pair<int, int> type; 
  mpz_class part_g; 
  fread(&part_bgraph_size, sizeof part_bgraph_size, 1, f);
  for (int i=0;i<part_bgraph_size;i++){
    // read t, t'
    fread(&t, sizeof t, 1, f);
    fread(&tp, sizeof tp, 1, f);
    type = pair<int, int>(t, tp);
    mpz_inp_raw(part_g.get_mpz_t(), f);
    part_bgraph.insert(pair<pair<int, int>, mpz_class> (type, part_g));
  }

  // === part graphs

  // first, the size
  int part_graph_size;
  int v_size;
  vector<int> W; 
  fread(&part_graph_size, sizeof part_graph_size, 1, f);
  for (int i=0;i<part_graph_size; i++){
    // first, the type
    fread(&t, sizeof t, 1, f);
    // then, the mpz part
    mpz_inp_raw(part_g.get_mpz_t(), f);
    // then, the vector size
    fread(&v_size, sizeof v_size, 1, f);
    W.resize(v_size);
    for (int j=0;j<v_size; j++){
      fread(&int_in, sizeof int_in, 1, f);
      W[j] = int_in;
    }
    part_graph.insert(pair<int, pair< mpz_class, vector< int > > >(t, pair<mpz_class, vector<int> >(part_g, W)));
  }
}

marked_graph_compressed marked_graph_encoder::encode(const marked_graph& G)
{
  logger::current_depth++;
  logger::add_entry("Init compressed", "");

  compressed.clear(); // reset the compressed variable before starting

  n = G.nu_vertices;
  compressed.n = n;
  compressed.h = h;
  compressed.delta = delta;

  logger::add_entry("Extact edge types", "");
  extract_edge_types(G);
  //cout << " edge types extracted " << endl;


  compressed.ver_type_list = C.ver_type_list; // 
  compressed.type_mark = C.M.message_mark;

  /*
  cout << " message list " << endl;
  for (int i=0;i<C.M.message_list.size();i++){
    cout << i << " : ";
    for (int j=0;j<C.M.message_list[i].size();j++)
      cout << C.M.message_list[i][j] << " ";
    cout << endl;
  }
  */

  logger::add_entry("Encode * vertices", "");
  encode_star_vertices(); // encode the list of vertices with at least one star edge connected to them
  //cout << " encoded star vertices " << endl;

  logger::add_entry("Encode * edges", "");
  encode_star_edges(); // encode edges with star types, i.e. those with half edge type L or larger
  //cout << " encoded star edges " << endl;

  logger::add_entry("Encode vertex types", "");
  encode_vertex_types(); // encode the sequences \f$\vec{\beta}, \vec{D}\f$, which is encoded in C.ver_type
  //cout << " encoded vertex types " << endl;

  logger::add_entry("Extract partition graphs", "");
  extract_partition_graphs(); // for equality types, we form simple unmarked graphs, and for inequality types, we form a bipartite graph
  //cout << " extracted partition graphs " << endl;
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

  logger::add_entry("Encode partition b graphs", "");
  encode_partition_bgraphs();
  //cout << " encoded partition bgraphs " << endl;

  logger::add_entry("Encode partition graphs", "");
  encode_partition_graphs();
  //cout << " encoded partition graphs " << endl;
  
  logger::current_depth--;
  return compressed;
}

void marked_graph_encoder::encode(const marked_graph& G, FILE* f){
  logger::add_entry("Encode","");
  marked_graph_compressed comp = encode(G);
  logger::add_entry("Write to binary file", "");
  comp.binary_write(f);
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
  logger::current_depth++;
  logger::add_entry("Extract messages", "");
  C = colored_graph(G, h, delta);
  //cerr << " number of types " << C.M.message_mark.size() << endl;
  is_star_vertex = C.is_star_vertex;
  star_vertices = C.star_vertices;
  logger::current_depth--;
}

void marked_graph_encoder::encode_star_vertices()
{
  // compress the is_star_vertex list
  time_series_encoder star_encoder(n);
  vector<int> is_star_vertex_int(is_star_vertex.size());
  for (int i=0;i<is_star_vertex.size();i++){
    if(is_star_vertex[i] == true)
      is_star_vertex_int[i] = 1;
    else
      is_star_vertex_int[i] = 0;
  }
  compressed.star_vertices = star_encoder.encode(is_star_vertex_int);
}

void marked_graph_encoder::encode_star_edges()
{
  int x, xp; // auxiliary mark variables
  int w; // auxiliary vertex variable
  int v; // auxiliary vertex variable 
  for (int k=0; k<star_vertices.size(); k++){ // iterating over star vertices
    v = star_vertices[k];
    for (int i=0;i<C.adj_list[v].size();i++){
      if (C.M.is_star_message[C.adj_list[v][i].second.first] or C.M.is_star_message[C.adj_list[v][i].second.second]){ // this is a star edge
        x = C.M.message_mark[C.adj_list[v][i].second.first]; // mark towards v
        xp = C.M.message_mark[C.adj_list[v][i].second.second]; // mark towards other endpoint
        w = C.adj_list[v][i].first; // the other endpoint of the edge
        if (x < xp){ // if x > xp, we only store this edge when visiting the other endpoint (w), since we do not want to express an edge twice
          if (compressed.star_edges.find(pair<int, int>(x,xp)) == compressed.star_edges.end()) // this pair does not exist
            compressed.star_edges[pair<int, int>(x,xp)].resize(star_vertices.size()); // open space for all star vertices 
          compressed.star_edges.at(pair<int, int>(x,xp))[k].push_back(w); // add w to the position of v (which is k)
        }
        if (x == xp and w > v){ // if w < v, we store this edge when visiting the other endpoint (w) to avoid storing and edge twice
          if (compressed.star_edges.find(pair<int, int>(x,xp)) == compressed.star_edges.end()) // not yet exist
            compressed.star_edges[pair<int, int>(x,xp)].resize(star_vertices.size()); // open space
          compressed.star_edges.at(pair<int, int>(x,xp))[k].push_back(w);
        }
      }
    }
  }
}

void marked_graph_encoder::find_part_index_deg()
{
  // extracting part_index and part_deg
  part_index.resize(n);
  for (int v =0; v<n; v++){
    for (map< pair< int, int >, int >::iterator it = C.deg[v].begin(); it != C.deg[v].end(); it++){
      if (part_deg.find(it->first) == part_deg.end()){
        // this pair has not been observed yet in the graph
        // so v is the first index node
        part_index[v][it->first] = 0;
        // the degree of v in the partition graph is indeed it->second
        part_deg[it->first] = vector<int>({it->second}); 
      }else{
        // there are currently part_deg[it->first].size() many elements there, and v is the last arrival one, so its index is equal to the number of existing nodes 
        part_index[v][it->first] = part_deg.at(it->first).size();
        // append degree of v, which is it->second
        part_deg.at(it->first).push_back(it->second); 
      }
    }
  }
}

void marked_graph_encoder::extract_partition_graphs()
{
  find_part_index_deg();

  // for t \leq t', part_adj_list[(t,t')] is the adjacency list of the partition graph t,t'. If t < t', this is the adjacency list of the left nodes, if t = t', this is the forward adjacency list of the partition graph.

  map<pair<int, int>, vector<vector<int> > > part_adj_list;
  int t, tp; // types 
  for (map<pair<int, int>, vector<int> >::iterator it = part_deg.begin(); it!= part_deg.end(); it++){
    // search over all type pairs in part_deg
    t = it->first.first;
    tp = it->first.second;
    // t < t': bipartite, t = t': simple. In both cases, 
    if (t <= tp)
      part_adj_list[it->first] = vector<vector<int> >(it->second.size());
  }

  // going over the edges in the graph and forming partition_adj_list
  int w, p, q; // auxiliary variables 
  for (int v =0; v<n; v++){
    for (int i=0;i<C.adj_list[v].size();i++){
      w = C.adj_list[v][i].first; // the other endpoint
      t = C.adj_list[v][i].second.first; // color towards v
      tp = C.adj_list[v][i].second.second; // color towards w
      if (C.M.is_star_message[t] == false and C.M.is_star_message[tp] == false){
        p = part_index[v].at(pair<int, int>(t,tp)); // the index of v in the t part of the t,tp partition graph
        //cerr << " p " << p << endl;
        q = part_index[w].at(pair<int, int>(tp, t)); // the index of w in the tp part of the t,tp partition graph
        //cerr << " q " << q << endl;
        if (t < tp)
          part_adj_list.at(pair<int, int>(t,tp))[p].push_back(q);
        if ((t == tp) and (q > p))
          part_adj_list.at(pair<int, int>(t,t))[p].push_back(q);
      }
    }
  }

  // using partition_adj_list in order to construct partition graphs
  if(logger::stat){
    *logger::stat_stream << " partition graphs size: " << endl;
    *logger::stat_stream << " ====================== " << endl;
  }
  for (map<pair<int, int>, vector<vector<int> > >::iterator it=part_adj_list.begin(); it!=part_adj_list.end();it++){
    t = it->first.first;
    tp = it->first.second;
    if (t<tp){
      part_bgraph[it->first] = b_graph(it->second, part_deg.at(pair<int, int>(t,tp)), part_deg.at(pair<int, int>(tp, t))); // left and right degree sequences are read from the part_deg map
      if (logger::stat){
        *logger::stat_stream << " bipartite: (" << part_deg.at(pair<int, int> (t,tp)).size() << " , " << part_deg.at(pair<int, int>(tp,t)).size() << ")" << endl;
      }
    }
    if (t == tp){
      part_graph[t] = graph(it->second, part_deg.at(pair<int, int>(t,t)));
      if (logger::stat){
        *logger::stat_stream << " simple: "  << part_deg.at(pair<int, int>(t,t)).size() << endl;
      }
    }
  }
}

void marked_graph_encoder::encode_partition_bgraphs()
{
  int t, tp;

  // compressing bipartite graphs 
  for (map<pair<int, int> , b_graph>::iterator it = part_bgraph.begin(); it!=part_bgraph.end(); it++){
    // the color components are t, tp
    t = it->first.first; 
    tp = it->first.second; 
    b_graph_encoder E(part_deg.at(pair<int, int>(t,tp)),part_deg.at(pair<int, int>(tp, t)));
    compressed.part_bgraph[pair<int, int>(t,tp)] = E.encode(it->second);
  }

}

void marked_graph_encoder::encode_partition_graphs()
{
  int t;

  // compressing graphs
  for (map<int, graph>::iterator it=part_graph.begin(); it!=part_graph.end(); it++){
    t = it->first; // the color is t,t
    graph_encoder E(part_deg.at(pair<int, int>(t,t)));
    compressed.part_graph[t] = E.encode(it->second);
  }
}









// ==================================================
// marked graph decoder
// ==================================================




marked_graph marked_graph_decoder::decode(const marked_graph_compressed& compressed)
{
  logger::current_depth++;
  logger::add_entry("Init", "");
  n = compressed.n;
  h = compressed.h;
  delta = compressed.delta;


  edges.clear(); // clear the edge list of the marked graph to be decoded
  vertex_marks.clear(); // clear the list of vertex marks of the marked graph to be decoded

  logger::add_entry("Decode * vertices", "");
  decode_star_vertices(compressed);
  //cerr << " decoded star vertices " << endl;

  logger::add_entry("Decode * edges", "");
  decode_star_edges(compressed);
  //cerr << " decoded star edges " << endl;

  logger::add_entry("Decode vertex types", "");
  decode_vertex_types(compressed);
  //cerr << " decoded vertex types " << endl;

  logger::add_entry("Decode partition graphs", "");
  decode_partition_graphs(compressed);
  //cerr << " decoded partition graphs " << endl;

  logger::add_entry("Decode partition b graphs", "");
  decode_partition_bgraphs(compressed);
  //cerr << " decoded partition b graphs " << endl;

  // now, reconstruct the original marked graphs by assembling the vertex marks and edge list
  logger::add_entry("Construct decoded graph", "");
  marked_graph G(n, edges, vertex_marks);

  logger::current_depth--;
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
  Deg.clear(); // refresh
  Deg.resize(n);

  vector<int> x; // auxiliary vector

  for (int v=0;v<n;v++){
    if (ver_type_int[v] >= compressed.ver_type_list.size())
      cerr << " Warning: marked_graph_decoder::decode_vertex_types ver_type_int[" << v << "] is out of range" << endl;
    x = compressed.ver_type_list[ver_type_int[v]];
    vertex_marks[v] =x[0]; // the mark of vertex v is the first element in the type list of this vertex
    // now, we extract Deg[v] by looking at batches of size 3 in x
    for (int i=1;i<x.size();i+=3){
      if (i+2 >= x.size())
        cerr << " Error: marked_graph_decoder::decode_vertex_types, the type of vertex " << v << " does not obey length constrains, i.e. it does not have length 1 + 3k " << endl;
          Deg[v][pair<int, int>(x[i],x[i+1])] = x[i+2]; // x[i] and x[i+1] are types, and x[i+2] is the count
    }
  }

  find_part_deg_orig_index(); // find part_deg and orig_index maps
}

void marked_graph_decoder::find_part_deg_orig_index()
{
  part_deg.clear();
  origin_index.clear();
  int t, tp; // types

  //cerr << " decoded deg : " << endl;
  for (int v=0;v<n;v++){
    //cerr << " v " << v << endl;
    for (map<pair<int, int>, int>::iterator it=Deg[v].begin(); it!=Deg[v].end(); it++){
      t = it->first.first;
      tp = it->first.second;
      //cerr << " t " << t << " tp " << tp << " : " << it->second << endl;
      if (part_deg.find(it->first) == part_deg.end()){
        // this is our first encounter with this type pair
        origin_index[it->first] = vector<int>({v}); // v is the first node in the t side of the (t,t') partition graph
        part_deg[it->first] = vector<int>({it->second}); // the degree in the partition graph is read from it->second
      }else{
        origin_index.at(it->first).push_back(v); // v is the next vertex observed with type t,t', so the vertex in the partition graph with index origin_index[it->first] has original index v 
        // append degree of v, which is it->second
        part_deg.at(it->first).push_back(it->second);
      }
    }
  }
}

void marked_graph_decoder::decode_partition_graphs(const marked_graph_compressed& compressed)
{
  int t; // the type corresponding to the partition graph
  vector<int> t_message; // the actual message corresponding to t
  int x; // the mark component associated to t
  pair< mpz_class, vector< int > > G_compressed; // the compressed form of the partition graph
  graph G; // the decoded partition graph
  vector<int> flist; // the forward adjacency list of a vertex in a partition graph
  int w; // vertex in partition graph
  int v_orig, w_orig; // the original index of vertices v and w
  int n_G; // the number of vertices of the partitioned graph 

  for(map< int, pair< mpz_class, vector< int > > >::const_iterator it=compressed.part_graph.begin(); it!=compressed.part_graph.end(); it++){
    t = it->first;
    x = compressed.type_mark[t]; // the mark component of t 

    G_compressed = it->second;
    // the degree sequence of the graph can be obtained from part_deg.at(pair<int,int>(t,t))
    graph_decoder D(part_deg.at(pair<int, int>(t,t)));
    //cerr << " part_graph t = " << t << " with " << part_deg.at(pair<int, int>(t,t)).size() << " vertices " << endl;
    n_G = part_deg.at(pair<int, int>(t,t)).size(); // the number of vertices in the partition graph is read from the size of its degree sequence
    G = D.decode(G_compressed.first, G_compressed.second);
    // for each edge in G, we should add an edge with mark pair (x,x) to the edge list of the marked graph
    for (int v=0;v<n_G;v++){
      flist = G.get_forward_list(v);
      v_orig = origin_index.at(pair<int, int>(t,t))[v]; // the index of v in the original graph 
      for (int i=0;i<flist.size();i++){
        w = flist[i]; // the other endpoint in the partition graph
        w_orig = origin_index.at(pair<int, int>(t,t))[w]; // the index of w in the original graph
        edges.push_back(pair<pair<int, int>, pair<int, int> >(pair<int, int>(v_orig,w_orig), pair<int, int> (x,x)));
      }
    }
  }
}



void marked_graph_decoder::decode_partition_bgraphs(const marked_graph_compressed& compressed)
{
  pair<int, int> c; // the pair of types
  int t, tp; // types
  int x, xp; // mark components of t and tp

  b_graph G; // the decoded partition bipartite graph
  int nl_G; // the number of left nodes in the partition graph G 
  vector<int> adj_list; // adj list of a vertex in a partition bipartite graph
  int w; // a right node
  int v_orig, w_orig; // the original index of vertices v and w in partition graphs 
  for (map<pair<int, int>, mpz_class>::const_iterator it = compressed.part_bgraph.begin(); it!=compressed.part_bgraph.end(); it++){
    c = it->first;
    t = c.first;
    tp = c.second;
    x = compressed.type_mark[t]; // the mark component of t
    xp = compressed.type_mark[tp]; // the mark component of tp 

    //cerr << " t " << t << " tp " << tp << endl;

    b_graph_decoder D(part_deg.at(pair<int, int>(t,tp)), part_deg.at(pair<int, int>(tp,t))); // the degree sequence of left nodes is precisely part_deg.at(pair<int, int>(t,tp)), while that of the right nodes is precisely part_deg.at(pair<int, int>(tp,t))
    //cerr << " decoder constructed " << endl;
    //cerr << " part graph t = "  << t << " t' = " << tp << " nl " << part_deg.at(pair<int, int>(t,tp)).size() << " nr = " << part_deg.at(pair<int, int>(tp,t)).size() << endl;
    G = D.decode(it->second);

    //cerr << " G decoded " << endl;
    nl_G = part_deg.at(pair<int, int>(t,tp)).size(); // the number of left nodes in G is obtained from the size of the degree sequence of left nodes
    
    for (int v=0;v<nl_G;v++){
      v_orig = origin_index.at(pair<int, int>(t,tp))[v];
      //cerr << " v " << v << " v_orig " << v_orig << endl;
      adj_list = G.get_adj_list(v);
      for (int i=0;i<adj_list.size();i++){
        w = adj_list[i];
        w_orig = origin_index.at(pair<int, int>(tp,t))[w]; // since w is a right node, we should read its original index through origin_index[(tp,t)]
        //cerr << " w " << w << " w_orig " << w_orig << endl;
        edges.push_back(pair<pair<int, int>, pair<int, int> >(pair<int, int>(v_orig,w_orig), pair<int, int>(x,xp)));
      }
    }
  }
}


