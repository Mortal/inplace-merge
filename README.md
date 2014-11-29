In-place merging with O(1) extra space according to
Huang and Langston, Practical in-place merging (1987).

Requires both inputs to be of size K^2 for some K.

About 50 times slower than `std::inplace_merge`
and about 3 times slower than `std::sort`.
