#include "nemu.h"
#include "cpu/reg.h"

paddr_t page_translate(vaddr_t vaddr, bool is_write) {
  Log("vaddr:0x%x", vaddr);
 
  paddr_t base = cpu.cr3.page_directory_base << 12;
  vaddr_t dir = (vaddr >> 22) << 2;
  paddr_t pd_addr = base & dir;
  Log("pd_addr:0x%x", pd_addr);
  PDE pd;
  pd.val = paddr_read(pd_addr, 4);
  Log("pd_val:0x%x", pd.val);
  assert(pd.present);

  base = pd.page_frame << 12;
  dir = ((vaddr >> 12) & 0x3ff) << 2;
  paddr_t pt_addr = base & dir;
  Log("pt_addr:0x%x", pt_addr);
  PTE pt;
  pt.val = paddr_read(pt_addr, 4);
  Log("pt_val:0x%x", pt.val);
  assert(pt.present);

  base = pt.page_frame << 12;
  dir = vaddr & 0xfff;
  paddr_t paddr = base & dir;
  Log("paddr:0x%x", paddr);

  pd.accessed = 1;
  paddr_write(pd_addr, 4, pd.val);

  if ((pt.accessed == 0) || (pt.dirty == 0 && is_write)){
    pt.accessed = 1;
    pt.dirty = 1;
  }
  paddr_write(pt_addr, 4, pt.val);

  return paddr;
}
