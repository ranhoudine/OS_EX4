//
// Created by ranho on 6/15/23.
//
#include "VirtualMemory.h"
#include "MemoryConstants.h"
#include "PhysicalMemory.h"
#include "Utilities.h"

#define ROOT_WIDTH (VIRTUAL_MEMORY_SIZE % OFFSET_WIDTH == 0 ? OFFSET_WIDTH : VIRTUAL_MEMORY_SIZE % PAGE_SIZE )

/*
 * Initialize the virtual memory.
 */
void VMinitialize ()
{
  nullify_frame (0);
}

/* Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread (uint64_t virtualAddress, word_t *value)
{
  // pseudocode: look for physical address of the page. if found - read the content from the required cell within it into
  // value. otherwise - load page from disk and do the same.
  uint64_t offset;
  uint64_t frame_to_read_from = get_to_frame(virtualAddress);
  if (frame_to_read_from >= NUM_FRAMES)
    return 0;
  offset = get_address_offset_in_level (virtualAddress, TABLES_DEPTH);
  PMread(pm_address (frame_to_read_from, offset), value);
  return 1; // todo - what is a failure?
}

/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite (uint64_t virtualAddress, word_t value)
{
  // pseudocode: look for the physical address of the page. if found - write value into the required cell within it. otherwise load page
  // from disk and do the same
  uint64_t offset;
  uint64_t frame_to_write_to = get_to_leaf (virtualAddress);
  if (frame_to_write_to >= NUM_FRAMES)
    return 0;
  offset = get_address_offset_in_level (virtualAddress, TABLES_DEPTH);
  PMwrite(pm_address (frame_to_write_to, offset), value);
  return 1; // todo - what is a failure?
}
