## Draft Code for the Borsuk hypothesis in BoolCube in dim=10 with diam=6#

$K_t$ stands for Clique on t vertexes

$K_4^1$ stands for Clique on 4 vertexes, $\oplus$ on vertexes equals 0

$K_4^2$ stands for Clique on 4 vertexes which is not $K_4^1$

Tasks split:
| TaskId      | Description |
| ----------- | ----------- |
| 1      | $\nexists{K_3}$       |
| 2   | $\exists{K_3}$        |
| 3      | $\exists{K_4} \bigwedge \nexists{K_4^{1}}$       |
| 4   | $\exists{K_4^{1}}$|
| 5      | $\exists{K_4^{1}}  \bigwedge  \nexists$ $K_5 \setminus e$ on base of $K_4^{1} $      |
| 6   | $\exists$ $K_5 \setminus e$ on base of $K_4^{1} $         |
| 7      | $\exists{K_5}$        |
| 8   | $\exists{K_6}$         |

Files contents description

| FileName      | TaskId | Comment | 
| ----------- | ----------- |----------- |
| For_Voronov_no_k4_0_1_2-Copy2.ipynb      | 2       | Non-setup code starts from: Ниже рабочая зона (Да), and lasts till the end of file     |
| For_Voronov_no_k4_0_1_2-Copy3.ipynb      | 2       | No comment   |
| For_Voronov_no_k4_0_1_2_small_pieces_and_finish.ipynb    | 2       | Covers edge cases when +0 / +1 / +2 / +3 didn't color     |
| Every_K4_second_type_quick_check.ipynb      | 3       | Ниже рабочая зона (Да)#   |
| Every_K5.ipynb      | 7       | No comment   |
| Every_K5_without_edge.ipynb      | 6       |  Non-setup code starts from: Ниже рабочая зона (Да), and lasts till the end of file   |
| Every_K5_without_edge_0_1_2.ipynb     | 6       | Non-setup code starts from: Ниже рабочая зона (Да), and lasts till the end of file |
| For_Voronov_is_K6_plus_one.ipynb      | 8       | Special vertex added in the beginning. +4 vertexes   |
| Research_for_K6.ipynb      | 8       | +0 / +1 / +2/ +3 |
