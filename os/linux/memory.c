#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <os/memory.h>

static unsigned char memory_blocks_for_mem(size_t how_much)
{
        unsigned char blocks = (unsigned char) (((how_much + 1) / 4096) >> 2);
        return blocks == 0 ? 1 : blocks;
}

void memory_init(memory* mem, size_t reserved)
{
        assert(mem);
        assert(reserved);

        memset(mem, 0, reserved);

        // First byte tell us how many blocks are available
        *(mem) = memory_blocks_for_mem(reserved);
}

void* memory_alloc(memory* from, size_t how_much)
{
        assert(from);
        assert(how_much);

        memory* head = from + sizeof(unsigned char);
        memory* block = head;

        unsigned char max_blocks = (unsigned char) *from;
        unsigned char required_blocks = memory_blocks_for_mem(how_much);

        while (*block) {
                block += (unsigned char) (*block) * 16384;
                max_blocks--;
        }

        assert(max_blocks >= required_blocks);

        // First byte stores how many blocks were used for this alloc 
        *block = required_blocks;
        return block + sizeof(unsigned char);
}

void memory_free(memory* block)
{
        assert(block);

        // This chunk of alloc memory no longer uses any blocks
        *(block - sizeof(unsigned char)) = 0;
}
