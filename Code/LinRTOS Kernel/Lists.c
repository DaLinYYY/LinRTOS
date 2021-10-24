/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-22 
 * @LastEditTime: 2021-07-26 19:50:15
 * @Description: 
 */

#include "Lists.h"


/**
 * @Author: YangSL
 * @Description: 初始化节点
 * @param {tNode} *node: 要初始化的节点
 */
void vNodeInit(tNode *node)
{
    node->nextNode = node;
    node->preNode  = node;
 
}

/**
 * @Author: YangSL
 * @Description: 初始化链表
 * @param {tList *} list ：要初始化的链表
 */
void vListInit(tList * list)
{
    list->headNode.nextNode  = &(list->headNode);
    list->headNode.preNode  = &(list->headNode);
    list->nodeCount = 0;
}


/**
 * @Author: YangSL
 * @Description: 返回链表中节点个数
 * @param {tList} list : 哪个链表
 */
uint32_t uGetListNodeCount(tList *list)
{
    return list->nodeCount;
}


/**
 * @Author: YangSL
 * @Description: 返回链表的第一个节点
 * @param {tList *} list
 */
tNode * tGetFirstNode(tList * list)
{
    tNode * node = (tNode *)0;

    if (list->nodeCount != 0)
    {
        node = list->headNode.nextNode;
    }
    return node;
}


/**
 * @Author: YangSL
 * @Description: 返回链表的最后一个节点
 * @param {tList *} list
 */
tNode * tGetLastNode(tList * list)
{
    tNode * node = (tNode *)0;

    if(list->nodeCount != 0)
    {
        node = list->headNode.preNode;
    }
    return node;
}


/**
 * @Author: YangSL
 * @Description: 返回指定链表的前一个节点
 * @param {tList *} list
 * @param {tNode *} node
 */
tNode * tGetListPre(tList * list, tNode * node)
{
    if(node->preNode == node)
    {
        return (tNode *)0;
    }
    else
    {
        return node->preNode;
    }
}


/**
 * @Author: YangSL
 * @Description: 返回指定链表的后一个节点
 * @param {tList *} list
 * @param {tNode *} node
 */
tNode * tGetListNext(tList * list, tNode * node)
{
    if(node->nextNode == node)
    {
        return (tNode *)0;
    }
    else
    {
        return node->nextNode;
    }
}


/**
 * @Author: YangSL
 * @Description: 将新节点插入到链表开头
 * @param {tList *} list
 * @param {tNode *} node
 */
void vListInsertHead(tList * list, tNode * node)
{
    node->nextNode = list->headNode.nextNode;   
    node->preNode = list->headNode.nextNode->preNode;             
    
    list->headNode.nextNode->preNode = node;    
    list->headNode.nextNode = node;             

    list->nodeCount++;
}


/**
 * @Author: YangSL
 * @Description: 将新节点插入到链表结尾
 * @param {tList *} list
 * @param {tNode *} node
 */
void vListInsertLast(tList * list, tNode * node)
{
    node->nextNode = &(list->headNode);            
    node->preNode = list->headNode.preNode;     
    
    list->headNode.preNode->nextNode = node;    
    list->headNode.preNode = node;              

    list->nodeCount++;
}


/**
 * @Author: YangSL
 * @Description: 将新的节点插入到指定节点后面
 * @param {tList *} list    : 要插入的链表
 * @param {tNode *} newNode : 要插入的新节点
 * @param {tNode *} toNode  : 目标地址
 */
void vListInsertNodeAfter(tList * list, tNode * newNode, tNode * toNode)
{
    newNode->nextNode = toNode->nextNode;
    newNode->preNode = toNode->nextNode->preNode;

    toNode->nextNode->preNode = newNode;
    toNode->nextNode = newNode;

    list->nodeCount++;
}


/**
 * @Author: YangSL
 * @Description: 移除链表的第一个节点
 * @param {tList *} list
 * @Return : 如果链表为空，返回0，否则的话，返回第1个结点
 */
tNode * tListRemoveFirst(tList * list)
{
    tNode * node = (tNode *)0;
    if(list->nodeCount != 0)
    {
        node = list->headNode.nextNode;

        list->headNode.nextNode->nextNode->preNode = &(list->headNode);    
        list->headNode.nextNode = list->headNode.nextNode->nextNode;    	 

        list->nodeCount--;
    }
    return node;
}


/**
 * @Author: YangSL
 * @Description: 移除链表最后一个节点
 * @param {tList *} list
 * @Return : 如果链表为空，返回0，否则的话，返回最后1个结点
 */
tNode *tListRemoveLast(tList * list)
{
    tNode * node  = (tNode *)0;
    if(list->nodeCount != 0)
    {
        node = list->headNode.preNode;

        list->headNode.preNode = list->headNode.preNode->preNode;       	
        list->headNode.preNode->preNode->nextNode = &(list->headNode);    

        list->nodeCount--;
    }
    return  node;
}


/**
 * @Author: YangSL
 * @Description: 移除链表中的指定节点
 * @param {tList *} list : 操作的链表
 * @param {tNode *} node ：要删除得节点
 */
void vListRemoveNode(tList * list, tNode * node)
{
    node->nextNode->preNode = node->preNode;    
    node->preNode->nextNode = node->nextNode;   

    // node->preNode = node;       
    // node->nextNode = node;      

    list->nodeCount--;
}


/**
 * @Author: YangSL
 * @Description: 移除链表中所有节点
 * @param {tList *} list
 */
void vListRemoveAll(tList * list)
{
    uint32_t count;
    tNode * nextNode;
        
    /* 遍历所有的结点 */
	  nextNode = list->headNode.nextNode;
    for (count = list->nodeCount; count != 0; count-- )
    {
    	/* 先纪录下当前结点，和下一个结点,必须纪录下一结点位置，因为在后面的代码中当前结点的next会被重置 */
        tNode * currentNode = nextNode;
        nextNode = nextNode->nextNode;
        
        /* 重置结点自己的信息 */
        currentNode->nextNode = currentNode;
        currentNode->preNode = currentNode;
    }
    
    list->headNode.nextNode = &(list->headNode);
    list->headNode.preNode = &(list->headNode);
    list->nodeCount = 0;
}


