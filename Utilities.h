//
// Created by ranho on 6/15/23.
//

#include <cstdint>
#include "MemoryConstants.h"
#ifndef _UTILITIES_H_
#define _UTILITIES_H_
/***
 * finds the physical address of the given virtual address. if not found returns -1
 * @param address
 * @return
 */
uint64_t calculate_cyc_dist (const uint64_t &page_swapped_in, uint64_t concatenate);
uint64_t get_page_of_virtual_address(uint64_t virtual_address);
uint64_t concatenate (uint64_t way_to_page, uint64_t offset);
uint64_t get_to_frame(uint64_t virtual_address);
/***
 *
 * @param address
 * @param tree_level
 * @return
 */
uint64_t get_address_offset_in_level(uint64_t address, uint16_t tree_level);

/***
 * checks if the page corresponding to the virtual address is strictly smaller than NUM_PAGES
 * @param virtual_address
 * @return
 */
bool is_page_index_valid(uint64_t virtual_address);
/***
 * finds the index of the next available frame (i.e frame containing only zero entries). if memory is full, finds the best frame to
 * overwrite. makes sure not to use calling_frame
 * @return
 */
uint64_t find_next_available_frame(const uint64_t &calling_frame, const uint64_t &page_swapped_in);
void nullify_frame(uint64_t frame);
/***
 * converts frame and offset to physical address
 * @param frame
 * @param offset
 * @return
 */
uint64_t pm_address(uint64_t frame, uint64_t offset);

/***
 * traverses the tree and sets the next available frame to next_frame
 * @param root
 * @param calling_frame
 */
void dfs (const uint64_t &calling_frame, const uint64_t &page_swapped_in_vm,
          uint64_t cur_frame_pm, uint8_t level, uint64_t way_to_page_vm,
          uint64_t *empty_frame, uint64_t *max_pm_index_frame,
          uint64_t *max_cyc_dist, uint64_t *farthest_father_pm,
          uint64_t *farthest_page_offset, uint64_t *evicted_page);
uint16_t get_num_of_children(uint16_t level);
bool is_frame_nullified (uint64_t frame);

void run_over_leaves (const uint64_t &page_swapped_in_vm, uint64_t cur_frame_pm,
                      uint64_t way_to_page_vm, uint64_t *max_pm_index_frame,
                      uint64_t *max_cyc_dist, uint64_t *farthest_father_pm,
                      uint64_t *farthest_page_offset, uint64_t *evicted_page);
#endif //_UTILITIES_H_
