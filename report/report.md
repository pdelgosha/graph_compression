# Discussions

## 2019-04-08 

Improved MP by avoiding doing things twice. Since the commit of 3/13 (8642e59), I have made the following modifications: 

1. Instead of using binary search tree for `message_dict`, a hash table is used to improved the speed. 
2. Avoid doing things multiple times: when a message is fond in the dictionary, instead of using the bracket operator to assign the corresponding message, one can use an iterator that is helpful in both checking whether the message exists in the dictionary and also this assignment. 

With these modifications, here is the result running time of the algorithm when applied to a Poisson random graph with 100,000 nodes, edge and vertex mark set size = 10, and average number of nodes connected to every vertex = 3 (average degree would be 6), h = 3, delta = 20:

<details>
<summary> Click here to open </summary>

```
|---Construct G () 2019-04-08 07:37:02 PM
 edges size 300041
 graph constructed
|---Encode () 2019-04-08 07:37:03 PM
|---|---Init compressed () 2019-04-08 07:37:03 PM
|---|---Extact edge types () 2019-04-08 07:37:03 PM
|---|---|---Extract messages () 2019-04-08 07:37:03 PM
|---|---|---|---graph_message::update_message init () 2019-04-08 07:37:03 PM
|---|---|---|---resizing messages () 2019-04-08 07:37:03 PM
|---|---|---|---initializing messages () 2019-04-08 07:37:03 PM
|---|---|---|---updating messages () 2019-04-08 07:37:03 PM
 total time to search in hash table: 0.769341
 total time to insert in hash table: 1.53727
 total time to modify vector m  0.799577
 total time to sort  0.0696137
|---|---|---|---* symmetrizing () 2019-04-08 07:37:09 PM
|---|---|---|---setting message_mark and is_star_message () 2019-04-08 07:37:09 PM
|---|---|---colored_graph::init init () 2019-04-08 07:37:09 PM
|---|---|---updating adj_list () 2019-04-08 07:37:10 PM
|---|---|---Find deg and ver_types () 2019-04-08 07:37:10 PM
|---|---Encode * vertices () 2019-04-08 07:37:11 PM
|---|---Encode * edges () 2019-04-08 07:37:14 PM
|---|---Encode vertex types () 2019-04-08 07:37:14 PM
|---|---Extract partition graphs () 2019-04-08 07:37:16 PM
|---|---Encode partition b graphs () 2019-04-08 07:37:19 PM
|---|---Encode partition graphs () 2019-04-08 07:37:21 PM
|---Decode () 2019-04-08 07:37:21 PM
|---|---Init () 2019-04-08 07:37:21 PM
|---|---Decode * vertices () 2019-04-08 07:37:21 PM
|---|---Decode * edges () 2019-04-08 07:37:24 PM
|---|---Decode vertex types () 2019-04-08 07:37:24 PM
|---|---Decode partition graphs () 2019-04-08 07:37:30 PM
|---|---Decode partition b graphs () 2019-04-08 07:37:30 PM
|---|---Construct decoded graph () 2019-04-08 07:37:34 PM
|---compare () 2019-04-08 07:37:35 PM
 successfully decoded the marked graph :D

|---Construct G (): 1.007256s [2.847803%]
|---Encode (): 18.077404s [51.110016%]
|---|---Init compressed (): 0.000786s [0.004350%]
|---|---Extact edge types (): 8.238246s [45.572063%]
|---|---|---Extract messages (): 6.366953s [77.285301%]
|---|---|---|---graph_message::update_message init (): 0.001501s [0.023580%]
|---|---|---|---resizing messages (): 0.177938s [2.794706%]
|---|---|---|---initializing messages (): 0.182283s [2.862949%]
|---|---|---|---updating messages (): 5.933976s [93.199608%]
|---|---|---|---* symmetrizing (): 0.035266s [0.553898%]
|---|---|---|---setting message_mark and is_star_message (): 0.035971s [0.564971%]
|---|---|---colored_graph::init init (): 0.224685s [2.727335%]
|---|---|---updating adj_list (): 0.393801s [4.780152%]
|---|---|---Find deg and ver_types (): 1.252763s [15.206671%]
|---|---Encode * vertices (): 2.368763s [13.103448%]
|---|---Encode * edges (): 0.000043s [0.000240%]
|---|---Encode vertex types (): 2.317106s [12.817693%]
|---|---Extract partition graphs (): 2.823593s [15.619463%]
|---|---Encode partition b graphs (): 2.143847s [11.859265%]
|---|---Encode partition graphs (): 0.182942s [1.011993%]
|---Decode (): 13.384299s [37.841259%]
|---|---Init (): 0.000024s [0.000179%]
|---|---Decode * vertices (): 2.418280s [18.068033%]
|---|---Decode * edges (): 0.000034s [0.000256%]
|---|---Decode vertex types (): 6.676574s [49.883629%]
|---|---Decode partition graphs (): 0.000054s [0.000400%]
|---|---Decode partition b graphs (): 3.761533s [28.104073%]
|---|---Construct decoded graph (): 0.527769s [3.943198%]
|---compare (): 2.900620s [8.200886%]
       35.38 real        33.98 user         0.73 sys
```

</details>


Here, extracting edge types takes 45% of the compression time, and updating messages takes 93% of extracting edge types. Updating messages takes 5.9s, among which 0.76s is for search in hash table, 1.5s is for insertion in hash table, and 0.8s for modifying vector m (the message vector). This totals to 3s, yet 2.9s remains which is not clear what happens. Maybe I should do a more search on this. I also thought of updating messages parallel since updating for different nodes are independent. So I spent some time last week to learn multi thread programming in C++. 


## 2019-04-09 8:53 PM

Realized that in updating messages, the variable `neighbor_messages` is defined in each iteration of the loop. I defined it outside the loop, reserved some space for it, and cleared it inside the loop. This made some improvement (around 0.1 - 0.2 s): 

<details>
<summary> before </summary>

```
---Construct G () 2019-04-09 05:03:30 PM
 edges size 299475
 graph constructed
|---Encode () 2019-04-09 05:03:31 PM
|---|---Init compressed () 2019-04-09 05:03:31 PM
|---|---Extact edge types () 2019-04-09 05:03:31 PM
|---|---|---Extract messages () 2019-04-09 05:03:31 PM
|---|---|---|---graph_message::update_message init () 2019-04-09 05:03:31 PM
|---|---|---|---resizing messages () 2019-04-09 05:03:31 PM
|---|---|---|---initializing messages () 2019-04-09 05:03:31 PM
|---|---|---|---updating messages () 2019-04-09 05:03:31 PM
 total time to search in hash table: 0.76763
 total time to insert in hash table: 1.55482
 total time to modify vector m  0.801063
 total time to sort  0.0744823
 total time to collect neighbor messages 1.05781
|---|---|---|---* symmetrizing () 2019-04-09 05:03:37 PM
|---|---|---|---setting message_mark and is_star_message () 2019-04-09 05:03:37 PM
|---|---|---colored_graph::init init () 2019-04-09 05:03:37 PM
|---|---|---updating adj_list () 2019-04-09 05:03:37 PM
|---|---|---Find deg and ver_types () 2019-04-09 05:03:38 PM
|---|---Encode * vertices () 2019-04-09 05:03:39 PM
|---|---Encode * edges () 2019-04-09 05:03:41 PM
|---|---Encode vertex types () 2019-04-09 05:03:41 PM
|---|---Extract partition graphs () 2019-04-09 05:03:44 PM
|---|---Encode partition b graphs () 2019-04-09 05:03:46 PM
|---|---Encode partition graphs () 2019-04-09 05:03:48 PM
|---Decode () 2019-04-09 05:03:49 PM
|---|---Init () 2019-04-09 05:03:49 PM
|---|---Decode * vertices () 2019-04-09 05:03:49 PM
|---|---Decode * edges () 2019-04-09 05:03:51 PM
|---|---Decode vertex types () 2019-04-09 05:03:51 PM
|---|---Decode partition graphs () 2019-04-09 05:03:57 PM
|---|---Decode partition b graphs () 2019-04-09 05:03:57 PM
|---|---Construct decoded graph () 2019-04-09 05:04:01 PM
|---compare () 2019-04-09 05:04:01 PM
 successfully decoded the marked graph :D

|---Construct G (): 0.945466s [2.729596%]
|---Encode (): 17.966131s [51.868896%]
|---|---Init compressed (): 0.000691s [0.003846%]
|---|---Extact edge types (): 8.346380s [46.456192%]
|---|---|---Extract messages (): 6.465446s [77.464073%]
|---|---|---|---graph_message::update_message init (): 0.001417s [0.021923%]
|---|---|---|---resizing messages (): 0.199745s [3.089419%]
|---|---|---|---initializing messages (): 0.187164s [2.894831%]
|---|---|---|---updating messages (): 6.012073s [92.987755%]
|---|---|---|---* symmetrizing (): 0.030768s [0.475881%]
|---|---|---|---setting message_mark and is_star_message (): 0.034264s [0.529948%]
|---|---|---colored_graph::init init (): 0.225783s [2.705163%]
|---|---|---updating adj_list (): 0.389235s [4.663514%]
|---|---|---Find deg and ver_types (): 1.265874s [15.166743%]
|---|---Encode * vertices (): 2.294833s [12.773107%]
|---|---Encode * edges (): 0.000052s [0.000290%]
|---|---Encode vertex types (): 2.315811s [12.889873%]
|---|---Extract partition graphs (): 2.749254s [15.302428%]
|---|---Encode partition b graphs (): 2.082472s [11.591098%]
|---|---Encode partition graphs (): 0.174702s [0.972397%]
|---Decode (): 12.768882s [36.864243%]
|---|---Init (): 0.000014s [0.000111%]
|---|---Decode * vertices (): 2.372627s [18.581320%]
|---|---Decode * edges (): 0.000034s [0.000268%]
|---|---Decode vertex types (): 6.345600s [49.695812%]
|---|---Decode partition graphs (): 0.000051s [0.000399%]
|---|---Decode partition b graphs (): 3.525432s [27.609560%]
|---|---Construct decoded graph (): 0.525092s [4.112280%]
|---compare (): 2.957078s [8.537197%]
       34.64 real        33.47 user         0.72 sys
```

</details>


<details>
<summary> after </summary>

```
|---Construct G () 2019-04-09 05:05:10 PM
 edges size 300470
 graph constructed
|---Encode () 2019-04-09 05:05:11 PM
|---|---Init compressed () 2019-04-09 05:05:11 PM
|---|---Extact edge types () 2019-04-09 05:05:11 PM
|---|---|---Extract messages () 2019-04-09 05:05:11 PM
|---|---|---|---graph_message::update_message init () 2019-04-09 05:05:11 PM
|---|---|---|---resizing messages () 2019-04-09 05:05:11 PM
|---|---|---|---initializing messages () 2019-04-09 05:05:11 PM
|---|---|---|---updating messages () 2019-04-09 05:05:11 PM
 total time to search in hash table: 0.755252
 total time to insert in hash table: 1.52036
 total time to modify vector m  0.79718
 total time to sort  0.07528
 total time to collect neighbor messages 0.752213
|---|---|---|---* symmetrizing () 2019-04-09 05:05:17 PM
|---|---|---|---setting message_mark and is_star_message () 2019-04-09 05:05:17 PM
|---|---|---colored_graph::init init () 2019-04-09 05:05:17 PM
|---|---|---updating adj_list () 2019-04-09 05:05:17 PM
|---|---|---Find deg and ver_types () 2019-04-09 05:05:18 PM
|---|---Encode * vertices () 2019-04-09 05:05:19 PM
|---|---Encode * edges () 2019-04-09 05:05:21 PM
|---|---Encode vertex types () 2019-04-09 05:05:21 PM
|---|---Extract partition graphs () 2019-04-09 05:05:24 PM
|---|---Encode partition b graphs () 2019-04-09 05:05:26 PM
|---|---Encode partition graphs () 2019-04-09 05:05:28 PM
|---Decode () 2019-04-09 05:05:29 PM
|---|---Init () 2019-04-09 05:05:29 PM
|---|---Decode * vertices () 2019-04-09 05:05:29 PM
|---|---Decode * edges () 2019-04-09 05:05:31 PM
|---|---Decode vertex types () 2019-04-09 05:05:31 PM
|---|---Decode partition graphs () 2019-04-09 05:05:38 PM
|---|---Decode partition b graphs () 2019-04-09 05:05:38 PM
|---|---Construct decoded graph () 2019-04-09 05:05:41 PM
|---compare () 2019-04-09 05:05:42 PM
 successfully decoded the marked graph :D

|---Construct G (): 0.956205s [2.775045%]
|---Encode (): 17.612391s [51.113735%]
|---|---Init compressed (): 0.000653s [0.003709%]
|---|---Extact edge types (): 7.961563s [45.204330%]
|---|---|---Extract messages (): 6.084527s [76.423775%]
|---|---|---|---graph_message::update_message init (): 0.001444s [0.023740%]
|---|---|---|---resizing messages (): 0.192311s [3.160649%]
|---|---|---|---initializing messages (): 0.187849s [3.087325%]
|---|---|---|---updating messages (): 5.632613s [92.572739%]
|---|---|---|---* symmetrizing (): 0.032790s [0.538907%]
|---|---|---|---setting message_mark and is_star_message (): 0.037503s [0.616363%]
|---|---|---colored_graph::init init (): 0.214541s [2.694708%]
|---|---|---updating adj_list (): 0.395865s [4.972202%]
|---|---|---Find deg and ver_types (): 1.266586s [15.908758%]
|---|---Encode * vertices (): 2.331242s [13.236377%]
|---|---Encode * edges (): 0.000055s [0.000311%]
|---|---Encode vertex types (): 2.293806s [13.023817%]
|---|---Extract partition graphs (): 2.750105s [15.614605%]
|---|---Encode partition b graphs (): 2.096790s [11.905195%]
|---|---Encode partition graphs (): 0.176885s [1.004324%]
|---Decode (): 12.922110s [37.501854%]
|---|---Init (): 0.000021s [0.000161%]
|---|---Decode * vertices (): 2.465586s [19.080364%]
|---|---Decode * edges (): 0.000037s [0.000283%]
|---|---Decode vertex types (): 6.388016s [49.434776%]
|---|---Decode partition graphs (): 0.000078s [0.000603%]
|---|---Decode partition b graphs (): 3.535769s [27.362165%]
|---|---Construct decoded graph (): 0.532570s [4.121383%]
|---compare (): 2.966536s [8.609321%]
       34.46 real        33.37 user         0.72 sys
```

</details>


## 2019-04-09 9:17 PM

Realized that it is not really necessary to work with `adj_location` which is a complex map structure in marked graphs. Instead, defined `index_in_neighbor`: for a vertex v, `index_in_neighbor[v][i]` is the index of v in the adjacency list of the ith neighbor of v.
