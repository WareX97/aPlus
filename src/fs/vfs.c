#include <aplus.h>
#include <aplus/fs.h>
#include <aplus/list.h>

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <dirent.h>
#include <errno.h>


static list_t* list_inodes = 0;
static uint32_t nextinode = 0;

inode_t* vfs_root; 


void vfs_map(inode_t* inode) {
	inode->ino = (ino_t) nextinode++;
	list_add(list_inodes, (listval_t) inode);
}

void vfs_umap(inode_t* inode) {
	list_remove(list_inodes, (listval_t) inode);
	kfree(inode);
}

inode_t* vfs_mapped(inode_t* parent, char* name) {
	list_foreach(tmp, list_inodes) {
		inode_t* value = (inode_t*) tmp;
		
		if(value->parent == parent)
			if(strcmp(value->name, name) == 0)
				return value;
	}
	
	return NULL;
}

inode_t* vfs_mapped_at_index(inode_t* parent, int index) {
	list_foreach(tmp, list_inodes) {
		inode_t* value = (inode_t*) tmp;
		
		if(value->parent == parent)
			index--;
			
		if(index == -1)
			return value;
	}
	
	return NULL;
}

int vfs_mapped_count(inode_t* parent) {
	int index = 0;
	
	list_foreach(tmp, list_inodes) {
		inode_t* value = (inode_t*) tmp;
		
		if(value->parent == parent)
			index++;
	}
	
	return index;
}


int vfs_chroot(inode_t* root) {
	vfs_root = root;
	vfs_root->parent = 0;
	
	return 0;
}



int vfs_init() {
	list_init(list_inodes);
	
	
	vfs_root = (inode_t*) kmalloc(sizeof(inode_t));
	vfs_root->name[0] = '/';
	vfs_root->name[1] = 0;
	
	vfs_root->dev = (dev_t) 0;
	vfs_root->ino = (ino_t) nextinode++;
	vfs_root->mode = S_IFDIR;
	vfs_root->nlink = 0;
	vfs_root->uid = UID_ROOT;
	vfs_root->gid = GID_ROOT;
	vfs_root->rdev = (dev_t) 0;
	vfs_root->size = (size_t) 0;
	//vfs_root->atime = vfs_root->ctime = vfs_root->mtime = time(NULL);
	vfs_root->parent = (inode_t*) NULL;
	vfs_root->link = (inode_t*) NULL;
	
	vfs_root->read = NULL;
	vfs_root->write = NULL;
	vfs_root->readdir = NULL;
	vfs_root->finddir = NULL;
	vfs_root->creat = NULL;
	vfs_root->rename = NULL;
	vfs_root->unlink = NULL;
	vfs_root->chown = NULL;
	vfs_root->flush = NULL;
	vfs_root->ioctl = NULL;
	
	list_add(list_inodes, (listval_t) vfs_root);
	
	return 0;
}