unsigned long computeQuodigious(unsigned long value, unsigned long sum, unsigned long product) {
  if ((value % sum) == 0 && ((value % product) == 0)) {
    return value;
  } else {
    return 0;
  }
}


kernel void vector_add(global unsigned char *sum, global unsigned long *product, global unsigned long *value, global unsigned long *result) {
    // Get the index of the current element
    int i = get_global_id(0);
    // compute the offset addresses
    result[(4 * i) + 0] = computeQuodigious(value[i] + 2, sum[i] + 2, product[i] * 2);
    result[(4 * i) + 1] = computeQuodigious(value[i] + 4, sum[i] + 4, product[i] * 4);
    result[(4 * i) + 2] = computeQuodigious(value[i] + 6, sum[i] + 6, product[i] * 6);
    result[(4 * i) + 3] = computeQuodigious(value[i] + 8, sum[i] + 8, product[i] * 8);
}
