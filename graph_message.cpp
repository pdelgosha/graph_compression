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
  int w;
  int my_location;
  messages.resize(nu_vertices);
  //inward_message.resize(nu_vertices);
  //message_dict.resize(h);
  //message_list.resize(h);
  //message_mark.resize(h);
  //is_star_message.resize(h);

  
  // initialize the messages

  logger::add_entry("resizing messages", "");
  for (int v=0;v<nu_vertices;v++){
    messages[v].resize(G.adj_list[v].size());
    //inward_messages[v].resize(G.adj_list[v].size());
    //for (int i=0;i<G.adj_list[v].size();i++){
    //messages[v][i].resize(h);
      //inward_messages[v][i].resize(h);
    //}
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
      send_message(m, v, i);

      // adding this message to the message dictionary
      //it = message_dict[0].find(m);
      //w = G.adj_list[v][i].first;

      /*
      if (it == message_dict[0].end()){
        message_dict[0][m] = message_list[0].size();
        messages[v][i][0] = message_list[0].size();
        message_list[0].push_back(m);

      }else{
        messages[v][i][0] = it->second;
      }
      */
      //messages[v][i][0] = message_dict[0][m]; // the message at time 0
    }
  }

  // these are copies of message_dict, message_mark and is_star_message at the previous step, which are used to update messages at the current step.

  unordered_map<vector<int>, int, vint_hash> message_dict_old;
  vector<int> message_mark_old;
  vector<bool> is_star_message_old;
  vector<vector<int> > messages_old;

  // updating messages
  logger::add_entry("updating messages", "");
  m.reserve(5+ 2 * Delta);
  vector<int> m2;
  m2.reserve(5 + 2*Delta); // an auxiliary message when we need to work with two types of messages simultaneously 
  duration<float> diff;
  high_resolution_clock::time_point t1, t2; 
  float agg_search = 0;
  float agg_insert = 0;
  float agg_m = 0;
  float agg_sort = 0;
  float agg_neigh_message = 0;

  vector<pair<pair<int, int>, int> > neighbor_messages; // the first component is the message and the second is the name of the neighbor
  // the second component is stored so that after sorting, we know the owner of the message
  neighbor_messages.reserve(5+2*Delta);

  int nu_star_neigh; // number of star neighbors, i.e. neighbors of a vertex v whose message towards v are star type
  int star_neigh_index; // the index of the star neighbor of v, this is only useful when there is one star neighbor, if there are more than one star neighbor, then the message sent from v towards all other neighbors are star typed
  int star_neigh; // the label of the star neighbor, i.e. star_neigh = G.adj_list[v][star_neigh_index].first;
  int previous_message; // the message from the previous step
  int mark_to_v; // mark towards the current vertex directed from its neighbor
  vector<int> neighbors_list; // the list of neighbors of a vertex in the order after sorting with respect to their corresponding messages
  neighbors_list.reserve(Delta + 3);
  int deg_v; // the degree of vertex v 



  for (int s=1; s<h;s++){ // s stands for step
    //cerr << endl << endl<< " depth " << s << endl;
    // store variables corresponding to the previous step in their old version, and clearing the variables for this step:
    message_dict_old = message_dict;
    messages_old = messages;
    message_mark_old = message_mark;
    is_star_message_old = is_star_message;
    message_dict.clear();
    message_mark.clear();
    is_star_message.clear();
    // we do not clear messages since we need its size to be the same, and we only modify its content
    

    for (int v=0;v<nu_vertices;v++){
      deg_v = G.adj_list[v].size();
      if (deg_v==1){
        // no need to collect messages, there is only one message towards the one neighbor, which is known 
        m.resize(3); // there is only one message which is of the form \f$(\theta, 0, x)\f$, where \f$\theta\f$ is the mark of v, and \f$x = \xi_G(w,v)\f$  where \f$w\f$ is the only neighbor of v
        m[0] = G.ver_mark[v];
        m[1] = 0;
        m[2] = G.adj_list[v][0].second.first;
        send_message(m, v, 0);
      }else{
        if (deg_v <= Delta){
          neighbor_messages.clear();
          nu_star_neigh = 0;
          for (int i=0;i<deg_v;i++){
            w = G.adj_list[v][i].first; // neighbor label
            my_location = G.index_in_neighbor[v][i];
            previous_message = messages_old[w][my_location]; // the message sent from this neighbor towards v at time t-1
            // check if previous message is star
            if (is_star_message_old[previous_message]){
              nu_star_neigh ++;
              star_neigh_index = i;
              star_neigh = w;
            }
            if (nu_star_neigh >= 2)
              break; // then message towards all neighbors will be star, no need to collect messages
            mark_to_v = G.adj_list[v][i].second.first;
            neighbor_messages.push_back(pair<pair<int, int> , int> (pair<int,int>(previous_message, mark_to_v), i));
          }
          if (nu_star_neigh == 2){
            // message towards all the neighbors will be star
            m.resize(2);
            m[0] = -1;
            for (int i=0;i<G.adj_list[v].size();i++){
              m[1] = G.adj_list[v][i].second.first; 
              send_message(m, v, i); // send message m from v towards its ith neighbor at step s
            }
          }
          if (nu_star_neigh == 1){
            // the message towards all the neighbors except for that star neighbor is star
            // let m be the message towards that neighbor and m2 be the star messages

            // sorting neighbor messages
            sort(neighbor_messages.begin(), neighbor_messages.end());

            // preparing m 
            m.resize(0);
            m.push_back(G.ver_mark[v]);
            m.push_back(G.adj_list[v].size()-1);

            

            for (int i=0;i<neighbor_messages.size();i++){
              if (neighbor_messages[i].second != star_neigh_index){
                // collect the messages of non star neighbors
                m.push_back(neighbor_messages[i].first.first);
                m.push_back(neighbor_messages[i].first.second);
              }
            }
            // finalize m by inserting its mark component
            m.push_back(G.adj_list[v][star_neigh_index].second.first);

            // prepare star messages
            m2.resize(2);
            m2[0] = -1;
            for (int i=0;i<deg_v;i++){
              if (i==star_neigh_index){
                // send the prepared message m
                send_message(m, v, i);
              }else{
                // prepare a star message and send it
                m2[1] = G.adj_list[v][i].second.first;
                send_message(m2, v, i);
              }
            }
          }

          if (nu_star_neigh == 0){
            // no star neighbor, so we can prepare messages to all neighbors comfortably as none of them are star type
            // we do this by a masking technique
            // sorting neighbor messages
            sort(neighbor_messages.begin(), neighbor_messages.end());
            if (neighbor_messages.size() != deg_v){
              cerr << " Error: no star messages and yet neighbor_messages does not have a size equal to the deg of v, step " << s << " v= " << v <<  " deg_v= " << deg_v << " neighbor_messages.size() " << neighbor_messages.size() << endl;
            }
            m.resize(1 + 1 + 2*(G.adj_list[v].size()-1) +1); // 1 for vertex mark, 1 for deg -1, for (deg-1) many neighbors, each we have 2 values, and finally 1 for the mark component
            m[0] = G.ver_mark[v];
            m[1] = G.adj_list[v].size()-1;
            neighbors_list.resize(deg_v);
            for (int i=0;i<neighbor_messages.size();i++){
              m[2*(i+1)]   = neighbor_messages[i].first.first;
              m[2*(i+1)+1] = neighbor_messages[i].first.second;
              neighbors_list[i] = neighbor_messages[i].second;
            }
            // swapping the last message so that its mark component comes first, so that we can treat m as a valid message in our standard
            swap(m[2*deg_v], m[2*deg_v+1]);

            for (int i=neighbor_messages.size()-1;i>=0;i--){
              if (i < neighbor_messages.size()-1){
                swap(m[2*deg_v], m[2*(i+1)+1]);
                swap(m[2*deg_v+1], m[2*(i+1)]);
                swap(neighbors_list[deg_v-1], neighbors_list[i]);
              }
              send_message(m,v,neighbors_list[deg_v-1]);
            }
          }
        }
        if (deg_v > Delta){ // the message towards all neighbors is star
          m.resize(2);
          m[0] = -1;
          for(int i=0;i<deg_v;i++){
            m[1] = G.adj_list[v][i].second.first;
            send_message(m,v,i);
          }
        }
      }
    }
  }


  logger::add_entry("* symmetrizing", "");
  bool star1, star2;
  m.resize(2); // prepare for star message
  m[0] = -1;
  for (int v=0;v<nu_vertices;v++){
    for (int i=0;i<G.adj_list[v].size();i++){
      w = G.adj_list[v][i].first;
      my_location = G.index_in_neighbor[v][i];
      if (w > v){ // to avoid going over edges twice 
        star1 = is_star_message[messages[v][i]];
        star2 = is_star_message[messages[w][my_location]];
        if (star1 and !star2){
          // messages[v][i] should also become star
          m[1] = G.adj_list[v][i].second.first;
          send_message(m,v,i);
        }
        if (!star1 and star2){
          // message[w][my_location] should be star
          m[1] = G.adj_list[v][i].second.second;
          send_message(m,w,my_location);
        }
      }
    }
  }
  logger::current_depth--;




  // ==================================================
  // ==================================================
  // ==================================================
  // ==================================================
  // ==================================================
  // ==================================================
  // ==================================================
  // ==================================================

  /*
  for (int t=1;t<h;t++){
    for (int v=0;v<nu_vertices;v++){
      //cerr << " vertex " << v << endl;
      if (G.adj_list[v].size() <= Delta){
        // the degree of v is no more than Delta
        // do the standard message passing by aggregating messages from neighbors
        // stacking all the messages from neighbors of v towards v
        neighbor_messages.clear();

        // the message from each neighbor of v, say w,  towards v is considered, the mark of the edge between w and v towards v is added to it, and then all these objects are stacked in neighbor_messages to be sorted and used afterwards
        //t1 = high_resolution_clock::now();
        for (int i=0;i<G.adj_list[v].size();i++){
          w = G.adj_list[v][i].first; // what is the name of the neighbor I am looking at now, which is the ith neighbor of vertex v 
          //my_location = G.adj_location[w].at(v); <--- the inefficient way
          my_location = G.index_in_neighbor[v][i];
          // where is the place of node v among the list of neighbors of the ith neighbor of v
          int previous_message = messages[w][my_location][t-1]; // the message sent from this neighbor towards v at time t-1
          int mark_to_v = G.adj_list[v][i].second.first;
          neighbor_messages.push_back(pair<pair<int, int> , int> (pair<int,int>(previous_message, mark_to_v), w));
        }
        //t2 = high_resolution_clock::now();
        //diff = t2 - t1;
        //agg_neigh_message += diff.count();

        //t1 = high_resolution_clock::now();
        sort(neighbor_messages.begin(), neighbor_messages.end()); // sorts lexicographically
        //t2 = high_resolution_clock::now();
        //diff = t2 - t1;
        //agg_sort += diff.count();

        for (int i=0;i<G.adj_list[v].size();i++){
          // let w be the current ith neighbor of v
          int w = G.adj_list[v][i].first;
          // first, start with the mark of v and the number of offsprings in the subgraph component of the message
          //vector<int> m; // the message that v is going to send to w
          //t1 = high_resolution_clock::now();
          m.clear();
          m.push_back(G.ver_mark[v]); // mark of v
          m.push_back(G.adj_list[v].size()-1); // the number of offsprings in the subgraph component of the message
          //t2 = high_resolution_clock::now();
          //diff = t2 - t1;
          //agg_m += diff.count();

          // stacking messages from all neighbors of v expect for w towards v at time t-1
          for (int j=0;j<G.adj_list[v].size();j++){
            if (neighbor_messages[j].second != w){
              if (message_list[t-1][neighbor_messages[j].first.first][0] == -1){
                // this means that one of the messages that should be aggregated is * typed, therefore the outgoing messages should also be * typed
                // i.e. the message has only two entries: (-1, \xi(w,v)) where \xi(w,v) is the mark of the edge between v and w towards v
                // since after this loop, the mark \xi(w,v) is added to the message (after the comment starting with 'finally'), we only add the initial -1 part
                //t1 = high_resolution_clock::now();
                m.resize(0);
                m.push_back(-1);
                //t2 = high_resolution_clock::now();
                //diff = t2 - t1;
                //agg_m += diff.count();
                break; // the message is decided, we do not need to go over any of the other neighbor messages, hence break
              }
              // this message should be added to the list of messages
              //t1 = high_resolution_clock::now();
              m.push_back(neighbor_messages[j].first.first); // message part
              m.push_back(neighbor_messages[j].first.second); // mark part towards v
              //t2 = high_resolution_clock::now();
              //diff = t2 - t1;
              //agg_m += diff.count();

            }
          }
          // if we break, we reach at this point and message is (-1), otherwise the message is of the form (\tau(v), \deg(v) - 1, ...) where ... is the list of all neighbor messages towards v except for w. 
          // finally, the mark of the edge between v and w towards v, \xi(w,v), should be added to this list
          //t1 = high_resolution_clock::now();
          m.push_back(G.adj_list[v][i].second.first);
          //t2 = high_resolution_clock::now();
          //diff = t2 - t1;
          //agg_m += diff.count();

          // set the current message
          //t1 = high_resolution_clock::now();
          it = message_dict[t].find(m);
          //t2 = high_resolution_clock::now();
          //diff = t2 - t1;
          //agg_search += diff.count();

          if (it == message_dict[t].end()){
            //t1 = high_resolution_clock::now();
            //message_dict[t][m] = message_list[t].size();
            message_dict[t].insert(pair<vector<int>, int> (m, message_list[t].size()));
            //t2 = high_resolution_clock::now();
            //diff = t2 - t1;
            //agg_insert += diff.count();

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
          //t1 = high_resolution_clock::now();
          m.clear();
          m.resize(2);
          m[0] = -1;
          m[1] = G.adj_list[v][i].second.first;
          //t2 = high_resolution_clock::now();
          //diff = t2 - t1;
          //agg_m += diff.count();

          // set the current message
          //t1 = high_resolution_clock::now();
          it = message_dict[t].find(m);
          //t2 = high_resolution_clock::now();
          //diff = t2 - t1;
          //agg_search += diff.count();

          if (it == message_dict[t].end()){
            //t1 = high_resolution_clock::now();
            //message_dict[t][m] = message_list[t].size();
            message_dict[t].insert(pair<vector<int>, int> (m, message_list[t].size()));
            //t2 = high_resolution_clock::now();
            //diff = t2 - t1;
            //agg_insert += diff.count();
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
  cerr << " total time to collect neighbor messages " <<  agg_neigh_message << endl;
  // now, we should update messages at time h-1 so that if the message from v to w is *, i.e. is of the form (-1,x), then the message from w to v is also of the similar form, i.e. it is (-1,x') where x' = \xi(v,w)
  logger::add_entry("* symmetrizing", "");
  for (int v=0;v<nu_vertices;v++){
    for (int i=0;i<G.adj_list[v].size();i++){
      if (message_list[h-1][messages[v][i][h-1]][0] == -1){
        // it is of the form *
        w = G.adj_list[v][i].first; // the other endpoint of the edge
        //my_location = G.adj_location[w].at(v); // so that adj_list[w][my_location].first = v
        my_location = G.index_in_neighbor[v][i];

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
  */
}


/*!
  sends a message by setting messages, and puts it in the message hash table message_dict. It also updates message_mark and is_star_message corresponding to step s and the input message.

  \param m: the message to be sent
  \param v: the vertex from which the message is originated
  \param i: the message is sent to the ith neighbor of v
  \param s: the step, or depth (\f$0 \lee s < h\f$)
 */
inline void graph_message::send_message(const vector<int>& m, int v, int i){
  unordered_map<vector<int>, int, vint_hash>::iterator it;
  //cerr << " send message (";
  //for (int k=0;k<m.size();k++){
  //  cerr << m[k];
  //  if (k<m.size()-1)
  //    cerr << ", ";
  // }
  //cerr << "): " << v << " -> " << i;

  it = message_dict.find(m);
  if (it == message_dict.end()){
    // this is a new message
    message_dict.insert(pair<vector<int>, int> (m, message_mark.size())); // insert the message into the hash table, message_mark[s].size() is in fact the number of registered marks at step s
    messages[v][i] = message_mark.size(); // set the message
    message_mark.push_back(m.back()); // register m by adding its mark component (which is m.back() the last element in m) to the list of marks at step s
    is_star_message.push_back(m[0]==-1); // check if m is star type, and add this information to the list
  }else{
    // the message already exists, just use the registered integer value corresponding to m and send the message
    messages[v][i] = it->second;
  }
  //cout << " message = " << messages[v][i] << endl;
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
  //adj_location = G.adj_location; // neighborhood structure is the same as the given graph
  index_in_neighbor = G.index_in_neighbor;

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
      //my_location = G.adj_location[w].at(v); // where v stands among the neighbors of w
      my_location = index_in_neighbor[v][i]; 
      color_v = M.messages[v][i]; // the color towards v corresponds to the message v sends to w
      color_w = M.messages[w][my_location]; // the color towards w is the message w sends towards v
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
