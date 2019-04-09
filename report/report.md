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
