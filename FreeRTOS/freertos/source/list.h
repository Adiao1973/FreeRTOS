#ifndef LIST_H
#define LIST_H
#include "FreeRTOSConfig.h"

/* 链表节点数据结构定义 */
struct xLIST_ITEM
{
    TickType_t xItemValue; /* 辅助值，用于帮助节点做顺序排序*/
    struct xLIST_ITEM * pxNext; /* 指向链表下一个节点 */
    struct xLIST_ITEM * pxPrevious; /* 指向链表前一个节点 */
    void * pvOwner; /* 指向拥有该节点的内核对象，通常是TCB */
    void * pvContainner; /* 指向该节点所在的链表 */
};
typedef struct xLIST_ITEM ListItem_t;

/* 链表精简节点结构体定义 */
struct xMINI_LIST_ITEM
{
    TickType_t xItemValue; /* 辅助值，用于帮助节点做升序排序 */ 
    struct xLIST_ITEM * pxNext; /* 指向链表下一个节点 */
    struct xLIST_ITEM * pxPrevious; /* 指向链表前一个节点 */
};
typedef struct xMINI_LIST_ITEM MiniListItem_t; /* 精简节点数据类型重定义 */

/* 链表根节点数据结构定义 */
struct xLIST
{
    UBaseType_t uxNumberOfItems; /* 链表节点计数器 */
    ListItem_t * pxIndex; /* 链表节点索引指针 */
    MiniListItem_t xListEnd; /* 链表最后一个节点 */
};

typedef struct xLIST List_t;

/* 链表节点初始化 */
void vListInitialiseItem( ListItem_t * const pxItem);

/* 链表根节点初始化 */
void vListInitialise(List_t * const pxList);

/* 将节点接入到链表的尾部 */
void vListInsertEnd(List_t * const pxList,ListItem_t * const pxNewListItem);

/* 将节点按照升序排列插入到链表 */
void vListInsert(List_t * const pxList, ListItem_t * const pxNewListItem);

/* 将节点从链表删除 */
UBaseType_t uxListRemove(ListItem_t * const pxItemToRemove);

/* 初始化节点的拥有者 */
#define listSET_LIST_ITEM_OWNER(pxListItem, pxOwner)\
    ((pxListItem)->pvOwner = (void*)(pxOwner))

/* 获取节点拥有者 */
#define listGET_LIST_ITEM_OWNER(pxListItem)\
    ((pxListItem)->pvOwner)

/* 初始化节点排序辅助值 */
#define listSET_LIST_ITEM_VALUE(pxListItem, xValue)\
    ((pxListItem)->xItemValue = (xValue))

/* 获取节点排序辅助值 */
#define listGET_ITEM_VALUE_VALUE(pxListItem)\
    ((pxListItem)->xItemValue)

/* 获取链表根节点的节点计数器的值 */
#define listGET_ITEM_VALUE_OF_HEAD_ENIRY(pxList)\
    (((pxList)->xListEnd).pxNext->xItemValue)

/* 获取链表的入口节点 */
#define listGET_HEAD_ENIRY(pxList)\
    (((pxList)->xListEnd).pxNext)

/* 获取节点的下一个节点 */
#define listGET_NEXT(pxListItem)\
    ((pxListItem)->pxNext)

/* 获取链表的最后一个节点 */
#define listGET_END_MARKER(pxList)\
    ((ListItem_t const *)(&((pxList)->xListEnd)))

/* 判断链表是否为空 */
#define listLIST_IS_EMPTY(pxList)\
    ((BaseType_t)((pxList)->unxNumberOfItems == (UBaseType_t)))

/* 获取链表的节点数 */
#define listCURRENT_LIST_LENGTH(pxList)\
    ((pxList)->uxNumberOfItems)


/* 获取链表下一个节点的OWNER,即TCB */
#define listGET_OWNER_OF_NEXT_ENIRY(pxTCB, pxList)                              \
{                                                                               \
    List_t * const pxConstList = (pxList);                                      \
    /* 节点索引指向链表第一个节点 */                                               \
    (pxConstList)->pxIndex = (pxConstList)->pxIndex->pxNext;                    \
    /* */                                                                       \
    if((void *)(pxConstList)->pxIndex == (void *)&((pxConstList)->xListEnd))    \
    {                                                                           \
        (pxConstList)->pxIndex = (pxConstList)->pxIndex->pxNext;                \
    }                                                                           \
    /* 获取节点的OWNER，即TCB */                                                 \
    (pxTCB) = (pxConstList)->pxIndex->pvOwner;                                  \
}

#endif /* LIST_H */