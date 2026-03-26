int removeElement(int* nums, int numsSize, int val) {
    
    int write_idx = 0;   

    for (int i = 0; i < numsSize; i++){
        if(nums[i] == val){
            continue;
        }
        else {nums[write_idx] = nums[i]; write_idx++; }
    }
    return write_idx;
}