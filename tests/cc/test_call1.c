#include "../../src/cc/bpf_helpers.h"

BPF_TABLE("prog", int, int, jump, 64);
BPF_TABLE("array", int, u64, stats, 64);

enum states {
  S_EOP = 1,
  S_ETHER,
  S_ARP,
  S_IP
};

BPF_EXPORT(parse_ether)
int parse_ether(struct __sk_buff *skb) {
  size_t cur = 0;
  size_t next = cur + 14;

  int key = S_ETHER;
  u64 *leaf = stats.lookup(&key);
  if (leaf) (*leaf)++;

  switch (bpf_dext_pkt(skb, cur + 12, 0, 16)) {
    case 0x0800: jump.call(skb, S_IP);
    case 0x0806: jump.call(skb, S_ARP);
  }
  jump.call(skb, S_EOP);
  return 1;
}

BPF_EXPORT(parse_arp)
int parse_arp(struct __sk_buff *skb) {
  size_t cur = 14;  // TODO: get from ctx
  size_t next = cur + 28;

  int key = S_ARP;
  u64 *leaf = stats.lookup(&key);
  if (leaf) (*leaf)++;

  jump.call(skb, S_EOP);
  return 1;
}

BPF_EXPORT(parse_ip)
int parse_ip(struct __sk_buff *skb) {
  size_t cur = 14;  // TODO: get from ctx
  size_t next = cur + 20;

  int key = S_IP;
  u64 *leaf = stats.lookup(&key);
  if (leaf) (*leaf)++;

  jump.call(skb, S_EOP);
  return 1;
}

BPF_EXPORT(eop)
int eop(struct __sk_buff *skb) {
  int key = S_EOP;
  u64 *leaf = stats.lookup(&key);
  if (leaf) (*leaf)++;
  return 1;
}