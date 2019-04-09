#include "graph_message.h"

size_t vint_hash::operator()(vector<int> const& v) const{
  return boost::hash_range(v.begin(),v.end());
}


  /*!
    The structure of messages is as follows. To simplify the notation, we use \f$M_k(v,w)\f$ to denote the message sent from \f$v\f$ towards \f$w\f$ at time step \f$k\f$, this is in fact messages[v][i][t] where i is the index of \f$w\f$ among neighbors of \f$v\f$.

    - For \f$k = 0\f$, we have \f$M_0(v,w) = (\tau_G(v), 0,\xi_G(w,v))\f$ where \f$\tau_G(v)\f$ is the mark of vertex \f$v\f$ and \f$\xi_G(w,v)\f$ denotes the mark of the edge between \f$v\f$ and \f$w\f$ towards \f$v\f$.
    - For \f$k > 0\f$, if the degree of \f$v\f$ is bigger than Delta, we have \f$M_k(v,w) = (-1,\xi_G(w,v))\f$.
    - Otherwise, we form the list \f$(s_u: u \sim_G v, u \neq w)\f$, where for \f$u \sim_G v, u \neq w\f$, we set
    \f$s_u = (M_{k-1}(u,v), \xi_G(u,v))\f$.
    - If for some \f$u \sim_G v, u \neq w\f$,
    the sequence \f$s_u\f$ starts with a -1, we set \f$M_k(v,w) = (-1,\xi_G(w,v))\f$.
    - Otherwise, we sort the sequences \f$s_u\f$ nondecreasingly  with
    respect to the lexicographic order  and set \f$s\f$ to be
    the concatenation of the sorted list. Finally, we set \f$M_k(v,w) =
    (\tau_G(v), \deg_G(v) -1, s, \xi_G(w,v))\f$.
   */

void graph_message::update_messages(const marked_graph& G)
{
  logger::current_depth++;
  logger::add_entry("graph_message::update_message init", "");
  int nu_vertices = G.nu_vertices;

  messages.resize(nu_vertices);
  message_dict.resize(h);
  message_list.resize(h);

  // initialize the messages

  logger::add_entry("resizing messages", "");
  for (int v=0;v<nu_vertices;v++){
    messages[v].resize(G.adj_list[v].size());
    for (int i=0;i<G.adj_list[v].size();i++){
      messages[v][i].resize(h);
    }
  }

  logger::add_entry("initializing messages","");
  vector<int> m(3);
  unordered_map<vector<int>, int, vint_hash>::iterator it;
  //map<vector<int>, int>::iterator it;

  for (int v=0;v<nu_vertices;v++){

    for (int i=0;i<G.adj_list[v].size();i++){
      // the message from v towards the ith neighbor (lets call is w) at time 0 has a mark component which is \xi(v,w) and a subtree component which is a single root with mark \tau(v). This is encoded as a message vector with size 3 of the form (\tau(v), 0,\xi(v,w)) where the last 0 indicates that there is no offspring.


      //vector<int> m;
      //m.clear();
      //m.push_back(G.ver_mark[v]);
      //m.push_back(0);
      //m.push_back(G.adj_list[v][i].second.first);

      m[0] = G.ver_mark[v];
      m[1] = 0;
      m[2] = G.adj_list[v][i].second.first;

      // adding this message to the message dictionary
      it = message_dict[0].find(m); 
      if (it == message_dict[0].end()){
        message_dict[0][m] = message_list[0].size();
        messages[v][i][0] = message_list[0].size();
        message_list[0].push_back(m);
      }else{
        messages[v][i][0] = it->second;
      }

      //messages[v][i][0] = message_dict[0][m]; // the message at time 0
    }
  }

  // updating messages
  logger::add_entry("updating messages", "");
  m.reserve(5+ 2 * Delta);

  duration<float> diff;
  high_resolution_clock::time_point t1, t2; 
  float agg_search = 0;
  float agg_insert = 0;
  float agg_m = 0;
  float agg_sort = 0;

  for (int t=1;t<h;t++){
    for (int v=0;v<nu_vertices;v++){
      //cerr << " vertex " << v << endl;
      if (G.adj_list[v].size() <= Delta){
        // the degree of v is no more than Delta
        // do the standard message passing by aggregating messages from neighbors
        // stacking all the messages from neighbors of v towards v
        vector<pair<pair<int, int>, int> > neighbor_messages; // the first component is the message and the second is the name of the neighbor
        // the second component is stored so that after sorting, we know the owner of the message

        // the message from each neighbor of v, say w,  towards v is considered, the mark of the edge between w and v towards v is added to it, and then all these objects are stacked in neighbor_messages to be sorted and used afterwards 
        for (int i=0;i<G.adj_list[v].size();i++){
          int w = G.adj_list[v][i].first; // what is the name of the neighbor I am looking at now, which is the ith neighbor of vertex v 
          int my_location = G.adj_location[w].at(v); // where is the place of node v among the list of neighbors of the ith neighbor of v
          int previous_message = messages[w][my_location][t-1]; // the message sent from this neighbor towards v at time t-1
          int mark_to_v = G.adj_list[v][i].second.first;
          neighbor_messages.push_back(pair<pair<int, int> , int> (pair<int,int>(previous_message, mark_to_v), w));
        }

        t1 = high_resolution_clock::now();
        sort(neighbor_messages.begin(), neighbor_messages.end()); // sorts lexicographically
        t2 = high_resolution_clock::now();
        diff = t2 - t1;
        agg_sort += diff.count();

        for (int i=0;i<G.adj_list[v].size();i++){
          // let w be the current ith neighbor of v
          int w = G.adj_list[v][i].first;
          // first, start with the mark of v and the number of offsprings in the subgraph component of the message
          //vector<int> m; // the message that v is going to send to w
          t1 = high_resolution_clock::now();
          m.clear();
          m.push_back(G.ver_mark[v]); // mark of v
          m.push_back(G.adj_list[v].size()-1); // the number of offsprings in the subgraph component of the message
          t2 = high_resolution_clock::now();
          diff = t2 - t1;
          agg_m += diff.count();

          // stacking messages from all neighbors of v expect for w towards v at time t-1
          for (int j=0;j<G.adj_list[v].size();j++){
            if (neighbor_messages[j].second != w){
              if (message_list[t-1][neighbor_messages[j].first.first][0] == -1){
                // this means that one of the messages that should be aggregated is * typed, therefore the outgoing messages should also be * typed
                // i.e. the message has only two entries: (-1, \xi(w,v)) where \xi(w,v) is the mark of the edge between v and w towards v
                // since after this loop, the mark \xi(w,v) is added to the message (after the comment starting with 'finally'), we only add the initial -1 part
                t1 = high_resolution_clock::now();
                m.resize(0);
                m.push_back(-1);
                t2 = high_resolution_clock::now();
                diff = t2 - t1;
                agg_m += diff.count();
                break; // the message is decided, we do not need to go over any of the other neighbor messages, hence break
              }
              // this message should be added to the list of messages
              t1 = high_resolution_clock::now();
              m.push_back(neighbor_messages[j].first.first); // message part
              m.push_back(neighbor_messages[j].first.second); // mark part towards v
              t2 = high_resolution_clock::now();
              diff = t2 - t1;
              agg_m += diff.count();

            }
          }
          // if we break, we reach at this point and message is (-1), otherwise the message is of the form (\tau(v), \deg(v) - 1, ...) where ... is the list of all neighbor messages towards v except for w. 
          // finally, the mark of the edge between v and w towards v, \xi(w,v), should be added to this list
          t1 = high_resolution_clock::now();
          m.push_back(G.adj_list[v][i].second.first);
          t2 = high_resolution_clock::now();
          diff = t2 - t1;
          agg_m += diff.count();

          // set the current message
          t1 = high_resolution_clock::now();
          it = message_dict[t].find(m);
          t2 = high_resolution_clock::now();
          diff = t2 - t1;
          agg_search += diff.count();

          if (it == message_dict[t].end()){
            t1 = high_resolution_clock::now();
            //message_dict[t][m] = message_list[t].size();
            message_dict[t].insert(pair<vector<int>, int> (m, message_list[t].size()));
            t2 = high_resolution_clock::now();
            diff = t2 - t1;
            agg_insert += diff.count();

            messages[v][i][t] = message_list[t].size();
            message_list[t].push_back(m);
          }else{
            messages[v][i][t] = it->second;
          }
        }
      }else{
        // if the degree of v is bigger than Delta, the message towards all neighbors is of the form *
        // i.e. message of v towards a neighbor w is of the form (-1, \xi(w,v)) where \xi(w,v) is the mark of the edge between v and w towards v
        for (int i=0;i<G.adj_list[v].size();i++){
          //vector<int> m; // the current message from v to ith neighbor
          t1 = high_resolution_clock::now();
          m.clear();
          m.resize(2);
          m[0] = -1;
          m[1] = G.adj_list[v][i].second.first;
          t2 = high_resolution_clock::now();
          diff = t2 - t1;
          agg_m += diff.count();

          // set the current message
          t1 = high_resolution_clock::now();
          it = message_dict[t].find(m);
          t2 = high_resolution_clock::now();
          diff = t2 - t1;
          agg_search += diff.count();

          if (it == message_dict[t].end()){
            t1 = high_resolution_clock::now();
            //message_dict[t][m] = message_list[t].size();
            message_dict[t].insert(pair<vector<int>, int> (m, message_list[t].size()));
            t2 = high_resolution_clock::now();
            diff = t2 - t1;
            agg_insert += diff.count();
            messages[v][i][t] = message_list[t].size();
            message_list[t].push_back(m);
          }else{
            messages[v][i][t] = it->second;
          }
        }
      }
    }
  }
  cerr << " total time to search in hash table: " << agg_search << endl;
  cerr << " total time to insert in hash table: " << agg_insert << endl;
  cerr << " total time to modify vector m  " << agg_m << endl;
  cerr << " total time to sort  " << agg_sort << endl;

  // now, we should update messages at time h-1 so that if the message from v to w is *, i.e. is of the form (-1,x), then the message from w to v is also of the similar form, i.e. it is (-1,x') where x' = \xi(v,w)
  logger::add_entry("* symmetrizing", "");
  for (int v=0;v<nu_vertices;v++){
    for (int i=0;i<G.adj_list[v].size();i++){
      if (message_list[h-1][messages[v][i][h-1]][0] == -1){
        // it is of the form *
        int w = G.adj_list[v][i].first; // the other endpoint of the edge
        int my_location = G.adj_location[w].at(v); // so that adj_list[w][my_location].first = v
        //vector<int> m;
        m.clear();
        m.resize(2);
        m[0] = -1;
        m[1] = G.adj_list[v][i].second.second; // the mark towards w
        if (message_dict[h-1].find(m) == message_dict[h-1].end()){
          message_dict[h-1][m] = message_list[h-1].size();
          message_list[h-1].push_back(m);
        }
        messages[w][my_location][h-1] = message_dict[h-1][m];
      }
    }
  }

  // setting message_mark and is_star_message
  logger::add_entry("setting message_mark and is_star_message", "");
  message_mark.resize(message_list[h-1].size());
  is_star_message.resize(message_list[h-1].size());
  for (int i=0;i<message_list[h-1].size();i++){
    message_mark[i] = message_list[h-1][i].back(); // the last element is the mark component
    is_star_message[i] = (message_list[h-1][i][0] == -1); // message is star type when the first element is -1
  }
  logger::current_depth--;
}







bool pair_compare(const pair<vector<int> , int>& a, const pair<vector<int>, int>& b){
  return a.first < b.first;
}




/*!

  - updates messages for M
  - updates adj_list
  - updates ver_type, ver_type_dict, ver_type_list, ver_type_int
  - to make sure, checks whether the sum of degree matrices is symmetric

 */
void colored_graph::init(const marked_graph& G)
{
  logger::add_entry("colored_graph::init init", "");
  nu_vertices = G.nu_vertices;
  adj_location = G.adj_location; // neighborhood structure is the same as the given graph
  // assigning edge colors based on the messages given by M
  //M.update_messages();
  adj_list.resize(nu_vertices);

  // updating adj_list
  logger::add_entry("updating adj_list", "");
  int w, my_location, color_v, color_w;
  for (int v=0;v<nu_vertices;v++){
    adj_list[v].resize(G.adj_list[v].size()); // the same number of neighbors here
    for (int i=0;i<G.adj_list[v].size();i++){
      w = G.adj_list[v][i].first; // the ith neighbor, the same as in G
      my_location = G.adj_location[w].at(v); // where v stands among the neighbors of w
      color_v = M.messages[v][i][h-1]; // the color towards v corresponds to the message v sends to w
      color_w = M.messages[w][my_location][h-1]; // the color towards w is the message w sends towards v
      adj_list[v][i] = pair<int, pair<int, int> >(w, pair<int, int>(color_v, color_w)); // add w as a neighbor, in the same order as in G, and add the colors towards v and w 
    }
  }

  // updating the vertex type sequence, dictionary and list, i.e. variables ver_type, ver_type_dict and ver_type_list
  // we also update ver_type_int

  // implement and update deg and type_vertex_list
  
  int m, mp; // pair of types

  logger::add_entry("Find deg and ver_types", "");
  deg.resize(nu_vertices);
  is_star_vertex.resize(nu_vertices);
  ver_type.resize(nu_vertices);
  ver_type_int.resize(nu_vertices);

  for (int v=0;v<nu_vertices;v++){
    is_star_vertex[v] = false; // it is false unless we figure out otherwise, see below
    for (int i=0;i<adj_list[v].size(); i++){
      m = adj_list[v][i].second.first;
      mp = adj_list[v][i].second.second;
      if (M.is_star_message[m] == false and M.is_star_message[mp] == false){
        // this edge is not star type
        if (deg[v].find(pair<int, int>(m, mp)) == deg[v].end()){
          // this does not exist, so create it, since this is the first edge, its value must be 1
          deg[v][pair<int, int>(m, mp)] = 1;
          //type_vertex_list[pair<int, int>(m, mp)].push_back(v); // this must be done when we see the type (m, mp) for the first time here, so as to avoid multiple placing of v in the list 
        }else{
          // the edge exists, we only need to increase it by one
          deg[v][pair<int, int>(m, mp)] ++;
        }
      }else{
        // this is a star type vertex
        is_star_vertex[v] = true;
      }
    }

    // check if it was star vertex
    if (is_star_vertex[v] == true)
      star_vertices.push_back(v);


    // now, we form the type of this vertex
    // the type of a vertex is a vector x as follows:
    // x[0] is the vertex mark of v
    // x[3k+1], x[3k+2], x[3k+3] = (m_k, mp_k, deg[v][(m_k, mp_k)]) where (m_k, mp_k) is the kt key present in the map deg[v]. Since deg[v] is a map, we read its elements in increasing order (lexicographic order for pairs (m, mp)), hence this list is on a 1-1 correspondence with the pair (\theta(v), D(v)) in the paper.
    vector<int> vt; // type of v
    vt.resize(1+3 * deg[v].size()); // motivated by the above explanation
    vt[0] = G.ver_mark[v]; // mark of v
    int k = 0; // current index of vt
    for (map<pair<int, int>, int>::iterator it = deg[v].begin(); it != deg[v].end(); it++){
      vt[++k] = it->first.first; // m
      vt[++k] = it->first.second; // mp
      vt[++k] = it->second;
    }

    ver_type[v] = vt;
    // find ver_type_int[v]
    if (ver_type_dict.find(vt) == ver_type_dict.end()){
      // this is a new type, so add it to the dictionary and the list 
      ver_type_dict[vt] = ver_type_list.size();
      ver_type_list.push_back(vt);
    }
    ver_type_int[v] = ver_type_dict[vt];
  }
}
