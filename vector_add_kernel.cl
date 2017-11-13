kernel void vector_add(global unsigned char *sum, global unsigned long *product, global unsigned long *value, global unsigned long *result) {
    
    // Get the index of the current element
    int i = get_global_id(0);
    if (((value[i] % product[i]) == 0) && ((value[i] % sum[i]) == 0)) {
      result[i] = value[i];
    } else {
      result[i] = 0;
    }
}
