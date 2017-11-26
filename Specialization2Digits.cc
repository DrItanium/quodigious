template<>
inline void innerBody<2>(std::ostream& stream, u64 sum, u64 product, u64 index, u64 depth) noexcept {
++depth;
innerBody<0>(stream, sum+ 4 + 2, product* 4 * 2, index + 24, depth);
innerBody<0>(stream, sum+ 8 + 2, product* 8 * 2, index + 28, depth);
innerBody<0>(stream, sum+ 2 + 3, product* 2 * 3, index + 32, depth);
innerBody<0>(stream, sum+ 6 + 3, product* 6 * 3, index + 36, depth);
innerBody<0>(stream, sum+ 4 + 4, product* 4 * 4, index + 44, depth);
innerBody<0>(stream, sum+ 8 + 4, product* 8 * 4, index + 48, depth);
innerBody<0>(stream, sum+ 4 + 6, product* 4 * 6, index + 64, depth);
innerBody<0>(stream, sum+ 8 + 6, product* 8 * 6, index + 68, depth);
innerBody<0>(stream, sum+ 2 + 7, product* 2 * 7, index + 72, depth);
innerBody<0>(stream, sum+ 6 + 7, product* 6 * 7, index + 76, depth);
innerBody<0>(stream, sum+ 4 + 8, product* 4 * 8, index + 84, depth);
innerBody<0>(stream, sum+ 8 + 8, product* 8 * 8, index + 88, depth);
innerBody<0>(stream, sum+ 2 + 9, product* 2 * 9, index + 92, depth);
innerBody<0>(stream, sum+ 6 + 9, product* 6 * 9, index + 96, depth);
}
