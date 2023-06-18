//
// Created by ranho on 6/15/23.
//

#include "Utilities.h"
#include "PhysicalMemory.h"

#define LEAST_SIGNIFICANT_PAGE_WIDTH_BITS_MASK (1 << OFFSET_WIDTH) - 1
#define NUM_BLOCKS_IN_ADDRESS CEIL(VIRTUAL_ADDRESS_WIDTH / OFFSET_WIDTH)
/***
 * gets the physical address of the leaf. if not found - returns -1
 * @param address
 * @return
 */
//uint64_t find_leaf (uint64_t address)
//{
//  uint64_t current_frame = 0;
//  for (int i = 0; i < TABLES_DEPTH; ++i) // TABLE DEPTH is TREE HEIGHT
//  {
//    uint64_t local_offset = get_address_offset_in_level (address, i);
//    uint64_t current_frame = next_node_frame (current_frame, local_offset);
//  }
//  return current_frame;
//}

//uint64_t next_node_frame (uint64_t frame, uint64_t offset)
//{
//  word_t cell_content;
//  PMread (pm_address (frame, offset), &cell_content);
//  if (cell_content != 0)
//  {
//    return (uint64_t) cell_content;
//  }
//  else
//  {
//    // todo bring page from hard drive:
//    uint64_t next_available_frame = find_next_available_frame (frame);
//    nullify_frame (next_available_frame);
//    PMwrite (pm_address (frame, offset), (word_t) next_available_frame);
//    return next_available_frame;
//  }
//}
uint64_t get_page_of_virtual_address (uint64_t virtual_address)
{
  return (virtual_address >> OFFSET_WIDTH);
}

uint64_t find_next_available_frame (const uint64_t &calling_frame, const uint64_t &page_swapped_in)
{
  uint64_t max_pm_index_frame = 0;
  uint64_t empty_frame = 0;
  uint64_t max_cyc_dist = 0;
  uint64_t zero = 0;
  uint64_t farthest_father_pm = 0;
  uint64_t farthest_page_offset = 0;
  static int number_of_calls = 0;
  static uint64_t max_pm_index_frame_static = 0;
  number_of_calls++;
  dfs (calling_frame, page_swapped_in,
       zero, (uint8_t) 0,
       zero, &empty_frame,
       &max_pm_index_frame, &max_cyc_dist,
       &farthest_father_pm,
       &farthest_page_offset);
  max_pm_index_frame_static = max_pm_index_frame;
  if (empty_frame)
    return empty_frame;
  if (max_pm_index_frame < NUM_FRAMES - 1)
    return max_pm_index_frame + 1;
  word_t freed_frame_pm;
  PMread (pm_address (farthest_father_pm, farthest_father_pm), &freed_frame_pm);
  PMwrite (  (farthest_father_pm, farthest_page_offset), 0);
  return freed_frame_pm;
}

bool is_frame_nullified (uint64_t frame)
{
  word_t child;
  for (uint64_t offset = 0; offset < OFFSET_WIDTH; ++offset)
  {
    PMread (pm_address (frame, offset), &child);
    if (child != 0)
      return false;
  }
  return true;
}

void dfs (const uint64_t &calling_frame, const uint64_t &page_swapped_in_vm,
          uint64_t cur_frame_pm, uint8_t level, uint64_t way_to_page_vm,
          uint64_t *empty_frame, uint64_t *max_pm_index_frame,
          uint64_t *max_cyc_dist, uint64_t *farthest_father_pm,
          uint64_t *farthest_page_offset)
{
  if (*empty_frame != 0)
    return;

  if (cur_frame_pm > *max_pm_index_frame)
    *max_pm_index_frame = cur_frame_pm;

  uint64_t dep = TABLES_DEPTH; // todo debugging
  if (level == TABLES_DEPTH)
  {
    // TABLE THAT POINT AT LEAVES
    run_over_leaves (page_swapped_in_vm, cur_frame_pm,
                     way_to_page_vm, max_pm_index_frame,
                     max_cyc_dist, farthest_father_pm,
                     farthest_page_offset);
    return;
  }
  uint64_t page_size = PAGE_SIZE; // todo debug
  uint64_t tree_depth= TABLES_DEPTH;
  word_t frame_of_child;
  // Finding empty tables
  for (uint64_t offset = 0; offset < PAGE_SIZE; ++offset)
  {
    PMread (pm_address (cur_frame_pm, offset), &frame_of_child);
    if (frame_of_child != 0)
    {
      if (frame_of_child != calling_frame && is_frame_nullified ((uint64_t) frame_of_child))
      {
        *empty_frame = frame_of_child;
        PMwrite (pm_address (cur_frame_pm, offset), 0);
        return;
      }
      dfs (calling_frame, page_swapped_in_vm,
           (uint64_t) frame_of_child, level+1,
           concatenate (way_to_page_vm, offset),
           empty_frame, max_pm_index_frame,
           max_cyc_dist, farthest_father_pm,
           farthest_page_offset);
    }
  }
}

void run_over_leaves (const uint64_t &page_swapped_in_vm, uint64_t cur_frame_pm,
                      uint64_t way_to_page_vm, uint64_t *max_pm_index_frame,
                      uint64_t *max_cyc_dist, uint64_t *farthest_father_pm,
                      uint64_t *farthest_page_offset)
{
  word_t child_pm;
  *max_pm_index_frame = cur_frame_pm;
  if (*max_pm_index_frame < cur_frame_pm)
    *max_pm_index_frame = cur_frame_pm;
  uint64_t cyc_dist = 0;
  for (uint64_t leaf = 0; leaf < PAGE_SIZE; ++leaf)
  {
    PMread (pm_address (cur_frame_pm, leaf), &child_pm);
    if (child_pm > *max_pm_index_frame)
      *max_pm_index_frame = child_pm;
    if (child_pm != 0)
    {
      cyc_dist = calculate_cyc_dist (page_swapped_in_vm, concatenate (way_to_page_vm, leaf));
      if (*max_cyc_dist < cyc_dist)
      {
        *max_cyc_dist = cyc_dist;
        *farthest_page_offset = leaf;
        *farthest_father_pm = cur_frame_pm;
      }
    }
  }
}

uint64_t calculate_cyc_dist (const uint64_t &page_swapped_in, uint64_t leaf)
{
  uint64_t abs_dist_leaf_to_page = page_swapped_in - leaf >= 0 ? page_swapped_in - leaf : leaf - page_swapped_in;
  if (NUM_PAGES - abs_dist_leaf_to_page > abs_dist_leaf_to_page)
    return abs_dist_leaf_to_page;
  return NUM_PAGES - abs_dist_leaf_to_page;
}
uint64_t concatenate (uint64_t way_to_page, uint64_t offset)
{
  return (way_to_page << OFFSET_WIDTH) | offset;
}

void nullify_frame (uint64_t frame)
{
  for (int i = 0; i < OFFSET_WIDTH; ++i)
  {
    PMwrite (pm_address (0, i), 0);
  }
}

uint64_t pm_address (uint64_t frame, uint64_t offset)
{
  return frame * PAGE_SIZE + offset;
}

uint64_t get_to_frame(uint64_t virtual_address){
  word_t pm_cell_content;
  uint64_t frame = 0;
  uint64_t offset = 0;
  uint64_t page_to_read = get_page_of_virtual_address (virtual_address);
  for (int i = 0; i <= TABLES_DEPTH; ++i)
  {
    uint64_t num_frames = NUM_FRAMES; // todo debugging
    offset = get_address_offset_in_level (virtual_address, i);
    PMread (pm_address (frame, offset), &pm_cell_content);
    if (pm_cell_content == 0)
    {
      auto next_frame = (word_t) find_next_available_frame (frame, page_to_read);
      nullify_frame (next_frame);
      PMwrite (pm_address (frame, offset), next_frame);
      pm_cell_content = next_frame;
    }
    frame = pm_cell_content;
  }
  return frame;
}

/***
 * converts a global page address to the level-local address
 * @param address   full address of the page
 * @param tree_level 0 = root, then each level is the next OFFSET_WIDTH
 * @return
 */
uint64_t get_address_offset_in_level (uint64_t address, uint16_t tree_level)
{
  unsigned int shift_levels = NUM_BLOCKS_IN_ADDRESS - 1 - tree_level;
  uint64_t res = address >> (shift_levels * OFFSET_WIDTH);
  res &= LEAST_SIGNIFICANT_PAGE_WIDTH_BITS_MASK;
  return res;
}