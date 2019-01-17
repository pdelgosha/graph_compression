#include "graph_message.h"

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

void graph_message::update_messages()
{
  int nu_vertices = G.nu_vertices;

  messages.resize(nu_vertices);

  // initialize the messages
  for (int v=0;v<nu_vertices;v++){
    messages[v].resize(G.adj_list[v].size());
    for (int i=0;i<G.adj_list[v].size();i++){
      // the message from v towards the ith neighbor (lets call is w) at time 0 has a mark component which is \xi(v,w) and a subtree component which is a single root with mark \tau(v). This is encoded as a message vector with size 3 of the form (\tau(v), 0,\xi(v,w)) where the last 0 indicates that there is no offspring.
      messages[v][i].resize(h);
      // initialize messages to be empty
      for (int t=0;t<h;t++)
        messages[v][i][t].resize(0);

      vector<int> m;
      m.push_back(G.ver_mark[v]);
      m.push_back(0);
      m.push_back(G.adj_list[v][i].second.first);
      messages[v][i][0] = m; // the message at time 0
    }
  }

  // updating messages
  for (int t=1;t<h;t++){
    for (int v=0;v<nu_vertices;v++){
      if (G.adj_list[v].size() <= Delta){
        // the degree of v is no more than Delta
        // do the standard message passing by aggregating messages from neighbors
        // stacking all the messages from neighbors of v towards v
        vector<pair<vector<int>, int> > neighbor_messages; // the first component is the message and the second is the name of the neighbor
        // the second component is stored so that after sorting, we know the owner of the message

        // the message from each neighbor of v, say w,  towards v is considered, the mark of the edge between w and v towards v is added to it, and then all these objects are stacked in neighbor_messages to be sorted and used afterwards 
        for (int i=0;i<G.adj_list[v].size();i++){
          int w = G.adj_list[v][i].first; // what is the name of the neighbor I am looking at now, which is the ith neighbor of vertex v 
          int my_location = G.adj_location[w].at(v); // where is the place of node v among the list of neighbors of the ith neighbor of v
          vector<int> previous_message = messages[w][my_location][t-1]; // the message sent from this neighbor towards v at time t-1
          previous_message.push_back(G.adj_list[v][i].second.first); // adding the mark towards v to the list
          neighbor_messages.push_back(pair<vector<int> , int> (previous_message, w));
        }

        sort(neighbor_messages.begin(), neighbor_messages.end(), pair_compare);
        for (int i=0;i<G.adj_list[v].size();i++){
          // let w be the current ith neighbor of v
          int w = G.adj_list[v][i].first;
          // first, start with the mark of v and the number of offsprings in the subgraph component of the message
          messages[v][i][t].push_back(G.ver_mark[v]); // mark of v
          messages[v][i][t].push_back(G.adj_list[v].size()-1); // the number of offsprings in the subgraph component of the message
          // stacking messages from all neighbors of v expect for w towards v at time t-1
          for (int j=0;j<G.adj_list[v].size();j++){
            if (neighbor_messages[j].second != w){
              if (neighbor_messages[j].first[0] == -1){
                // this means that one of the messages that should be aggregated is * typed, therefore the outgoing messages should also be * typed
                // i.e. the message has only two entries: (-1, \xi(w,v)) where \xi(w,v) is the mark of the edge between v and w towards v
                // since after this loop, the mark \xi(w,v) is added to the message (after the comment starting with 'finally'), we only add the initial -1 part
                messages[v][i][t].resize(0);
                messages[v][i][t].push_back(-1);
                break; // the message is decided, we do not need to go over any of the other neighbor messages, hence break
              }
              // this message should be added to the list of messages
              messages[v][i][t].insert(messages[v][i][t].end(), neighbor_messages[j].first.begin(), neighbor_messages[j].first.end());
            }
          }
          // if we break, we reach at this point and message is (-1), otherwise the message is of the form (\tau(v), \deg(v) - 1, ...) where ... is the list of all neighbor messages towards v except for w. 
          // finally, the mark of the edge between v and w towards v, \xi(w,v), should be added to this list
          messages[v][i][t].push_back(G.adj_list[v][i].second.first);
        }
      }else{
        // if the degree of v is bigger than Delta, the message towards all neighbors is of the form *
        // i.e. message of v towards a neighbor w is of the form (-1, \xi(w,v)) where \xi(w,v) is the mark of the edge between v and w towards v
        for (int i=0;i<G.adj_list[v].size();i++){
          messages[v][i][t].resize(2);
          messages[v][i][t][0] = -1;
          messages[v][i][t][1] = G.adj_list[v][i].second.first;
        }
      }
    }
  }

  // now, we should update messages at time h-1 so that if the message from v to w is *, i.e. is of the form (-1,x), then the message from w to v is also of the similar form, i.e. it is (-1,x') where x' = \xi(v,w)
  for (int v=0;v<nu_vertices;v++){
    for (int i=0;i<G.adj_list[v].size();i++){
      if (messages[v][i][h-1][0] == -1){
        // it is of the form *
        int w = G.adj_list[v][i].first; // the other endpoint of the edge
        int my_location = G.adj_location[w].at(v); // so that adj_list[w][my_location].first = v
        messages[w][my_location][h-1].resize(2);
        messages[w][my_location][h-1][0] = -1;
        messages[w][my_location][h-1][1] = G.adj_list[v][i].second.second; // the mark towards w
      }
    }
  }
  update_message_dictionary(); // update the variables message_dict and message_list
}

/*!
  The message_list is sorted in reverse order so that all * messages (those messages starting with -1) go to the end of the list.
*/

void graph_message::update_message_dictionary()
{
  vector<int> message;
  for (int v=0;v<G.nu_vertices;v++){
    for (int i=0;i<G.adj_list[v].size();i++){
      message = messages[v][i][h-1];
      if(message_dict.find(message) == message_dict.end()){
        // the message does not exist in the dictionary, hence add it
        message_dict[message] = message_list.size(); // so that it points to the last element in message_list which is going to be added in the next line, this assures that if message_dict[m] = i, then message_list[i] = m
        message_list.push_back(message); // add the message to the list
      }
    }
  }

  // we want all the * messages to be together so that later we can easily distinguish between * messages and normal messages.
  // in order to do this, we simply sort the message list
  sort (message_list.begin(), message_list.end());
  // but, since we want the * messages which start by -1 to go to the end of the list, after sorting, we reverse the vector as well
  reverse(message_list.begin(), message_list.end());
  // then, we update message_dict accordingly
  // at the same time, we count the number of non * messages, i.e. L
  //L = 0;
  for (int i=0;i<message_list.size();i++){
    message_dict[message_list[i]] = i;
    //if (message_list[i][0] != -1)
    //L++;
  }
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
void colored_graph::init()
{
  nu_vertices = G.nu_vertices;
  adj_location = G.adj_location; // neighborhood structure is the same as the given graph
  // assigning edge colors based on the messages given by M
  M.update_messages();
  adj_list.resize(nu_vertices);

  // updating adj_list
  int w, my_location, color_v, color_w;
  for (int v=0;v<nu_vertices;v++){
    adj_list[v].resize(G.adj_list[v].size()); // the same number of neighbors here
    for (int i=0;i<G.adj_list[v].size();i++){
      w = G.adj_list[v][i].first; // the ith neighbor, the same as in G
      my_location = G.adj_location[w].at(v); // where v stands among the neighbors of w
      color_v = M.message_dict[M.messages[v][i][h-1]]; // the color towards v corresponds to the message v sends to w
      color_w = M.message_dict[M.messages[w][my_location][h-1]]; // the color towards w is the message w sends towards v
      adj_list[v][i] = pair<int, pair<int, int> >(w, pair<int, int>(color_v, color_w)); // add w as a neighbor, in the same order as in G, and add the colors towards v and w 
    }
  }

  // updating the vertex type sequence, dictionary and list, i.e. variables ver_type, ver_type_dict and ver_type_list
  // we also update ver_type_int

  int L = M.message_list.size(); // the number of messages
  ver_type.resize(nu_vertices);
  ver_type_int.resize(nu_vertices);
  for (int v=0;v<nu_vertices;v++){
    ver_type[v].resize(1 + L * L);
    ver_type[v][0] = G.ver_mark[v];
    for (int i=0;i<adj_list[v].size();i++){
      //if (adj_list[v][i].second.first < M.message_list.size()){ // equivalently, the edge is not * typed, since all * typed messages are after L by sorting
      ver_type[v][1 +  adj_list[v][i].second.first * L + adj_list[v][i].second.second] ++;
      //}
    }
    if (ver_type_dict.find(ver_type[v]) == ver_type_dict.end()){
      ver_type_list.push_back(ver_type[v]);
      ver_type_dict[ver_type[v]] = ver_type_list.size() -1;
    }
    ver_type_int[v] = ver_type_dict[ver_type[v]];
  }

  // checking whether the sum of degrees is symmetric
  vector<int> sum;
  sum.resize(1 + L * L);
  for (int v=0;v<nu_vertices;v++)
    for (int i=0;i<1 + L * L;i++)
      sum[i] += ver_type[v][i];
  for (int i=0;i<L;i++){
    for (int j=0;j<L;j++){
      cout << sum[1+i*L + j] << " ";
      if (sum[1+i*L + j] != sum[1+j*L+i])
        cout << " DANGER! the sum matrix is not symmetric" << endl;
    }
    cout << endl;
  }
}
