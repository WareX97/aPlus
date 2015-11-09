#include <xdev.h>
#include <xdev/module.h>
#include <xdev/vfs.h>
#include <libc.h>

MODULE_NAME("char/urandom");
MODULE_DEPS("");
MODULE_AUTHOR("WareX");
MODULE_LICENSE("GPL");


static int urandom_read(struct inode* inode, void* buf, size_t size) {
	char* bc = (char*) buf;
	size_t i;
	for(i = 0; i < size; i++)
		*bc++ = rand() % 256;

	return size;
}

int init(void) {
	inode_t* ino;
	if(unlikely((ino = vfs_mkdev("urandom", -1, S_IFCHR | 0666)) == NULL))
		return E_ERR;


	ino->read = urandom_read;
	return E_OK;
}



int dnit(void) {
	return E_OK;
}