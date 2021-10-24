/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-22
 * @LastEditTime: 2021-07-22 22:53:20
 * @Description: 
 */

#ifndef _LINOS_LISTS_H
#define _LINOS_LISTS_H
#include <stdint.h>

typedef struct  _tNode
{
    struct _tNode * preNode;
    struct _tNode * nextNode;
}tNode;

typedef struct _tList
{
    tNode   headNode;
    uint32_t nodeCount;
    
}tList;

typedef enum
{
    eListStatusOK = 0,
    eListStatusErr,
    eListStatusEmpty
}eListStatus;


/**
 * @Author: YangSL
 * @Description: 初始化节点
 * @param {tNode} *node
 */
void vNodeInit(tNode *node);


/**
 * @Author: YangSL
 * @Description: 初始化链表
 * @param {tList *} list
 */
void vListInit(tList * list);


/**
 * @Author: YangSL
 * @Description: 
 * @param {tList} list
 */
uint32_t uGetListNodeCount(tList *list);


/**
 * @Author: YangSL
 * @Description: 返回链表的第一个节点
 * @param {tList *} list
 */
tNode * tGetFirstNode(tList * list);


/**
 * @Author: YangSL
 * @Description: 返回链表的最后一个节点
 * @param {tList *} list
 */
tNode * tGetLastNode(tList * list);


/**
 * @Author: YangSL
 * @Description: 返回指定链表的前一个节点
 * @param {tList *} list
 * @param {tNode *} node
 */
tNode * tGetListPre(tList * list, tNode * node);


/**
 * @Author: YangSL
 * @Description: 返回指定链表的后一个节点
 * @param {tList *} list
 * @param {tNode *} node
 */
tNode * tGetListNext(tList * list, tNode * node);


/**
 * @Author: YangSL
 * @Description: 将新节点插入到链表开头
 * @param {tList *} list
 * @param {tNode *} node
 */
void vListInsertHead(tList * list, tNode * node);


/**
 * @Author: YangSL
 * @Description: 将新节点插入到链表结尾
 * @param {tList *} list
 * @param {tNode *} node
 */
void vListInsertLast(tList * list, tNode * node);


/**
 * @Author: YangSL
 * @Description: 将新的节点插入到指定节点后面
 * @param {tList *} list
 * @param {tNode *} node
 */
void vListInsertNodeAfter(tList * list, tNode * newNode, tNode * toNode);


/**
 * @Author: YangSL
 * @Description: 移除链表的第一个节点
 * @param {tList *} list :
 * @Return : 如果链表为空，返回0，否则的话，返回第1个结点
 */
tNode *tListRemoveFirst(tList * list);


/**
 * @Author: YangSL
 * @Description: 移除链表最后一个节点
 * @param {tList *} list
 * @Return : 如果链表为空，返回0，否则的话，返回第1个结点
 */
tNode *tListRemoveLast(tList * list);


/**
 * @Author: YangSL
 * @Description: 移除链表中的指定节点
 * @param {tList *} list
 * @param {tNode *} node
 */
void vListRemoveNode(tList * list, tNode * node);


/**
 * @Author: YangSL
 * @Description: 移除链表中所有节点
 * @param {tList *} list
 */
void vListRemoveAll(tList * list);

#endif /* _LINOS_LISTS_H */
