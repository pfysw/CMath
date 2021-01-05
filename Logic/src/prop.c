/*
 * prop.c
 *
 *  Created on: Aug 29, 2019
 *      Author: Administrator
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include <assert.h>
#include "prop.h"
#include "propseq.h"
#include "db.h"

typedef struct PermData
{
    u8 nAll;
    u8 iNofree;
    u8 nFree;
    u8 pos;
    u8 aMap[10];
}PermData;

DebugFlag gDebug = {
        .mpLeftDebug = 0,
        .deduceDebug = 0
};

u8 isEqualNode(TokenInfo *pA,TokenInfo *pB)
{
    u8 rc = 0;
    //if(!strcmp(pA->zSymb,pB->zSymb))//CopyAstTree并没拷贝zSymb
    //演绎定理中既有数字又有字母，只把字母设为相同
    if(pA->symb==pB->symb && pA->symb>'9')
    {
        rc = 1;
    }

    return rc;
}

int SetSameNode(
        AstParse *pParse,
        TokenInfo **ppAst,
        TokenInfo **ppTemp)
{
    int i;
    int rc;

    if( !pParse->cnt )
    {
        pParse->n = 0;
    }
    pParse->cnt++;
    if(  (*ppAst)->type==PROP_SYMB )
    {
        rc = 0;
        for( i=0;i<pParse->n;i++ )
        {
            if( isEqualNode((*ppAst),ppTemp[i]) )
            {
                if(*ppAst!=ppTemp[i])
                {
                    if(!pParse->usePool){
                        FreeAstNode(pParse,(*ppAst));
                    }
                    *ppAst = ppTemp[i];
                }
                rc = 1;
                break;
            }
        }
        if( !rc )
        {
            ppTemp[pParse->n++] = *ppAst;
        }
    }
    else
    {
        SetSameNode(pParse,&((*ppAst)->pLeft),ppTemp);
        if( (*ppAst)->type==PROP_IMPL )
            SetSameNode(pParse,&((*ppAst)->pRight),ppTemp);
    }
    pParse->cnt--;
    if(!pParse->cnt){
        pParse->usePool = 0;
    }
    return pParse->n;
}

int GetDiffNode(
        AstParse *pParse,
        TokenInfo **ppAst,
        TokenInfo **ppTemp,
        u8 isSubst)
{
    static int n = 0;
    static int cnt = 0;
    int i;
    int rc;
    TokenInfo *p;

    if( !cnt )
    {
        n = 0;
    }
    cnt++;
    if( cnt>11 )
    {
        pParse->bDiscard = 1;
    }
    if(  (*ppAst)->type==PROP_SYMB )
    {

        if( (*ppAst)->bSubst && isSubst &&
                ((*ppAst)->pSubst->type!=PROP_SYMB  ||
                        (*ppAst)->pSubst->bSubst ) )
        {
            cnt--;
            GetDiffNode(pParse,&((*ppAst)->pSubst),ppTemp,isSubst);
            cnt++;
        }
        else
        {

            rc = 0;
            if( (*ppAst)->bSubst && isSubst )
            {
                p = (*ppAst)->pSubst;
            }
            else
            {
                p = (*ppAst);
            }

            for( i=0;i<n;i++ )
            {
                if( p == ppTemp[i] )
                {
                    rc = 1;
                    break;
                }
            }
            if( !rc )
            {
                ppTemp[n++] = p;
            }

        }

    }
    else
    {
        GetDiffNode(pParse,&((*ppAst)->pLeft),ppTemp,isSubst);
        if( (*ppAst)->type==PROP_IMPL )
            GetDiffNode(pParse,&((*ppAst)->pRight),ppTemp,isSubst);
    }
    cnt--;
    return n;
}

int GetAllNode(
        AstParse *pParse,
        TokenInfo **ppAst,
        TokenInfo **ppTemp)
{
    static int n = 0;
    static int cnt = 0;

    if( !cnt )
    {
        n = 0;
    }
    cnt++;
    if(  (*ppAst)->type==PROP_SYMB )
    {

        if( (*ppAst)->bSubst && (*ppAst)->pSubst->type!=PROP_SYMB )
        {
            GetAllNode(pParse,&((*ppAst)->pSubst),ppTemp);
        }
        else
        {
            n++;
        }

    }
    else
    {
        GetAllNode(pParse,&((*ppAst)->pLeft),ppTemp);
        if( (*ppAst)->type==PROP_IMPL )
            GetAllNode(pParse,&((*ppAst)->pRight),ppTemp);
    }
    cnt--;
    return n;
}



void SubstFreeTerm(
        AstParse *pParse,
        TokenInfo *pAst,
        TokenInfo **ppTemp,
        PermData data)
{
    int i,j,t;
    PermData para;
    para.pos = data.pos+1;
    para.nAll = data.nAll;

    assert( para.pos<10 );
    if( 0==data.nFree )
    {
        PrintAst(pParse,pAst);
        return;
    }


    for(i=1; i<(1<<data.nFree); i++)
    {
        para.nFree = 0;
        if( !(i&1) )
        {
            continue;
        }
        for(j=0; j<data.nFree; j++)
        {
            if( (1<<j)&i )
            {
                t = data.aMap[j];
                ppTemp[t]->symb = 'A'+data.pos;
            }
            else
            {
                para.aMap[para.nFree] = data.aMap[j];
                para.nFree++;
            }
        }
        SubstFreeTerm(pParse,pAst,ppTemp,para);
    }
}

void SubstNofreeTerm(
        AstParse *pParse,
        TokenInfo *pAst,
        TokenInfo **ppTemp,
        PermData data)
{
    int i,j;

    assert( data.pos>0 );
    if( (1<<data.nAll)-1==data.iNofree )
    {
        SubstFreeTerm(pParse,pAst,ppTemp,data);
        return;
    }

    for(j=0; j<data.nAll; j++)
    {
        if( !((1<<j)&data.iNofree) )//找到一个非自由变元的位置
        {
            data.iNofree |= (1<<j);
            for(i=0; i<data.pos; i++)
            {
                ppTemp[j]->symb = 'A'+i;//在这个位置上遍历所有非自由变元
                SubstNofreeTerm(pParse,pAst,ppTemp,data);
            }
            break;
        }
    }
}

void PermPropSymb(
        AstParse *pParse,
        TokenInfo *pAst,
        TokenInfo **ppTemp,
        int nAll,
        int pos)
{
    int i,j;
    PermData data;

    memset(&data,0,sizeof(data));
    data.nAll = nAll;
    data.pos = pos;
    if( pos>0 )
    {
        for(i=0; i<(1<<nAll); i++)
        {
            memset(data.aMap,0,sizeof(data.aMap));
            data.nFree = 0;
            data.iNofree = i;
            for(j=0; j<nAll; j++)
            {
                if( (1<<j)&i )
                {
                    data.aMap[data.nFree] = j;
                    data.nFree++;
                }
            }
            SubstNofreeTerm(pParse,pAst,ppTemp,data);
        }
    }
    else
    {
        memset(data.aMap,0,sizeof(data.aMap));
        data.nFree = 0;
        for(j=0; j<nAll; j++)
        {
            data.aMap[data.nFree] = j;
            data.nFree++;
        }
        SubstFreeTerm(pParse,pAst,ppTemp,data);
    }
}

void GenBasicProp(AstParse *pParse)
{
    TokenInfo *pNode[5];
    TokenInfo *ppTemp[10];
    int i;
    int n;

    for(i=0;i<5;i++)
    {
        pNode[i] = NewNode(pParse);
        pNode[i]->symb = 'A'+i;
        pNode[i]->type = PROP_SYMB;
        //log_a("i %d %s",i,pNode[i]->zSymb);
        PrintAst(pParse,pNode[i]);
    }
    log_a("test1");
    PrintAst(pParse,pParse->pRoot);

    //PrintAstAddr(pParse,pParse->pRoot);
    log_a("set same");
    n = SetSameNode(pParse,&pParse->pRoot,ppTemp);
    log_a("n %d",n);
    PermPropSymb(pParse,pParse->pRoot,ppTemp,n,1);
    //PrintAstAddr(pParse,pParse->pRoot);
}

int isChildProp(AstParse *pParse,TokenInfo *pProp,TokenInfo *pSym)
{
    int rc = 0;

    assert( pSym->type==PROP_SYMB );

    if( pProp->type==PROP_SYMB )
    {
        if( pProp->bSubst && pProp->pSubst->type!=PROP_SYMB )
        {
            return isChildProp(pParse,pProp->pSubst,pSym);
        }

        if( pProp==pSym )
        {
            return 1;
        }
        else if( pProp->bSubst )
        {
            return isChildProp(pParse,pProp->pSubst,pSym);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        rc = isChildProp(pParse,pProp->pLeft,pSym);
        if( !rc && pProp->type==PROP_IMPL )
        {
            rc = isChildProp(pParse,pProp->pRight,pSym);
        }
    }
    return rc;
}
//把pA表示成pB的形式
int  SubstProp(
        AstParse *pParse,
        TokenInfo *pA,
        TokenInfo *pB)
{
    int rc = 0;

    while( pA->bSubst )
    //if( pA->bSubst && pA->pSubst->type!=PROP_SYMB )
    {
        pA = pA->pSubst;
    }
    while( pB->bSubst )
    //if( pB->bSubst && pB->pSubst->type!=PROP_SYMB )
    {
        pB = pB->pSubst;
    }

    if( pA->type==PROP_SYMB )
    {
        if( pB->type==PROP_SYMB )
        {
            //if( !pA->bSubst && pA!=pB )
            if( !pA->bSubst && !isChildProp(pParse,pB,pA) )
            {
                pA->pSubst = pB;
                pA->bSubst = 1;
            }
//            if( !pB->bSubst )
//            {
//                pB->pSubst = pA;
//                pB->bSubst = 1;
//            }

            return 1;
        }
        else
        {
            if( isChildProp(pParse,pB,pA) )
            {
                return 0;
            }
            else
            {
                pA->pSubst = pB;
                pA->bSubst = 1;
                rc = 1;
            }
        }
    }
    else
    {
        if( pB->type==PROP_SYMB )
        {
            if( isChildProp(pParse,pA,pB) )
            {
                return 0;
            }
            else
            {
                pB->pSubst = pA;
                pB->bSubst = 1;
                rc = 1;
            }
        }
        else if( pB->type!=pA->type )
        {
            return 0;
        }
        else
        {
            rc = SubstProp(pParse,pA->pLeft,pB->pLeft);
            if( rc&&pA->type==PROP_IMPL )
            {
                rc = SubstProp(pParse,pA->pRight,pB->pRight);
            }
        }
    }
    return rc;
}

void InsertVector(Vector *pV,TokenInfo *pData)
{

    if( pV->n==2978 )
    {
        log_a("ss");
    }
    if( pV->n<pV->size )
    {
        pV->data[pV->n++] = pData;
    }
    else
    {
        assert( pV->size<1000000 );

        pV->size = pV->size*2;
        pV->data = realloc(pV->data,sizeof(TokenInfo **)*pV->size);
        pV->data[pV->n++] = pData;
    }
}

void FreeVector(AstParse *pParse,Vector *pV)
{
    int i;
    for(i=0; i<pV->n; i++)
    {
        FreeAstTree(pParse,&pV->data[i],pParse->ppTemp);
    }
    pV->n = 0;
    free(pV->data);
    pParse->free_cnt++;
    free(pV);
    pParse->free_cnt++;
}

void ClearSubstFlag(AstParse *pParse,TokenInfo *pAst)
{
    assert(pAst!=NULL);

    if( pAst->type==PROP_SYMB )
    {
        pAst->bSubst = 0;
    }
    else if( pAst->type==PROP_NEG )
    {
        pAst->bSubst = 0;
        ClearSubstFlag(pParse,pAst->pLeft);
    }
    else
    {
        assert(pAst->type==PROP_IMPL);
        pAst->bSubst = 0;
        ClearSubstFlag(pParse,pAst->pLeft);
        ClearSubstFlag(pParse,pAst->pRight);
    }
}

int isSubstFlag(AstParse *pParse,TokenInfo *pAst)
{
    assert(pAst!=NULL);
    int rc = 0;

    if( pAst->type==PROP_SYMB )
    {
        rc = pAst->bSubst;
    }
    else if( pAst->type==PROP_NEG )
    {
        rc = isSubstFlag(pParse,pAst->pLeft);
    }
    else
    {
        assert(pAst->type==PROP_IMPL);
        rc = isSubstFlag(pParse,pAst->pLeft);
        if(!rc){
            rc = isSubstFlag(pParse,pAst->pRight);
        }
    }
    return rc;
}

#define DEBUG 1
#define INDEX_I   75
#define INDEX_J   15
#define NUM_NOT_SAME   4
#define NUM_ALL_NODE   27
#define LOOP_N         5


u8 aCnt[1000] = {0};

Vector *InitTheoremSet(AstParse *pParse)
{
    Vector *pSet = (Vector *)malloc(sizeof(Vector));
    pParse->malloc_cnt++;
    memset(pSet,0,sizeof(Vector));
    pSet->size = 100;
    pSet->data = malloc(pSet->size*sizeof(TokenInfo **));
    pParse->malloc_cnt++;
    return pSet;
}

TokenInfo *  PropMpSubst(
        AstParse *pParse,
        TokenInfo *pA,//条件
        TokenInfo *pB)//定理
{
    int rc = 0;
    int k;
    TokenInfo *apCopy[5] = {0};
    TokenInfo **ppTemp = pParse->ppTemp;
    int mxDiff = 7;
    int mxAll = 27;
    SetSameNode(pParse,&pA,ppTemp);
    SetSameNode(pParse,&pB,ppTemp);
    if(pA==pB)
    {
        if( pB->type!=PROP_IMPL )
        {
            goto end_insert;//不可能出现这种情况
        }
        apCopy[0] = CopyAstTree(pParse,pB,0);
        SetSameNode(pParse,&apCopy[0],ppTemp);
        rc = SubstProp(pParse,apCopy[0],pB->pLeft);
    }
    else
    {
        rc = SubstProp(pParse,pA,pB->pLeft);
    }

    if( rc )
    {
        int n,m;

        m = GetAllNode(pParse,&pB->pRight,ppTemp);
        if( m>mxAll ) {
            printf("GetAllNode num %d\n",m);
            goto end_insert;
        }
        n = GetDiffNode(pParse,&pB->pRight,ppTemp,1);
        if( n>mxDiff || pParse->bDiscard )
        {
            printf("GetDiffNode %d bDiscard %d\n",n,pParse->bDiscard);
            pParse->bDiscard = 0;
            goto end_insert;
        }
        for(k=0; k<n; k++)
        {
            ppTemp[k]->copy = 'A'+k;
        }
        apCopy[1] = CopyAstTree(pParse,pB->pRight,1);
        SetSameNode(pParse,&apCopy[1],ppTemp);
       // PrintSubstAst(pParse,apCopy[1]);
    }
    else
    {
        log_a("MpSubst fail");
    }

end_insert:
    ClearSubstFlag(pParse,pA);
    ClearSubstFlag(pParse,pB);
    if( pA==pB )
    {
        FreeAstTree(pParse,&apCopy[0],ppTemp);
    }

    return apCopy[1];
}

TokenInfo *  SubstMpLeft(
        AstParse *pParse,
        TokenInfo *pA,//定理
        TokenInfo *pB)//推论
{
    int rc = 0;
    int k;
    TokenInfo *apCopy[5] = {0};
    TokenInfo **ppTemp = pParse->ppTemp;
    int mxDiff = 7;
    int mxAll = 27;
    SetSameNode(pParse,&pA,ppTemp);
    SetSameNode(pParse,&pB,ppTemp);
    assert(pA!=pB);
    if(pB->type==PROP_IMPL){
        rc = SubstProp(pParse,pB->pRight,pA);
    }

    if( rc )
    {
        int n,m;
        if(gDebug.mpLeftDebug){
            PrintSubstAst(pParse,pA);
            PrintSubstAst(pParse,pB);
        }
        if(isSubstFlag(pParse,pA)){
            if(gDebug.mpLeftDebug){
                printf("has subst\n");
            }
            goto end_insert;
        }
        m = GetAllNode(pParse,&pB->pLeft,ppTemp);
        if( m>mxAll ) {
            printf("LeftNode num %d\n",m);
        }
        n = GetDiffNode(pParse,&pB->pLeft,ppTemp,1);
        if( n>mxDiff || pParse->bDiscard )
        {
            printf("GetDiffNode %d bDiscard %d\n",n,pParse->bDiscard);
            pParse->bDiscard = 0;
            assert(0);
            goto end_insert;
        }
        for(k=0; k<n; k++)
        {
            ppTemp[k]->copy = 'A'+k;
        }
        apCopy[1] = CopyAstTree(pParse,pB->pLeft,1);
        SetSameNode(pParse,&apCopy[1],ppTemp);
        if(gDebug.mpLeftDebug){
            printf("gen left\n");
        }
    }
    else
    {
        if(gDebug.mpLeftDebug){
            log_a("MpSubstLeft fail");
        }
    }

end_insert:
    ClearSubstFlag(pParse,pA);
    ClearSubstFlag(pParse,pB);
    return apCopy[1];
}


void  SubstSingleTest(AstParse *pParse,TokenInfo **ppTest)
{
    int i;
    int n;
    int rc;
    TokenInfo *ppTemp[100];
    for(i=0; i<3; i++)
    {
        PrintAst(pParse,ppTest[i]);
        n = SetSameNode(pParse,&ppTest[i],ppTemp);
        log_a("n %d",n);
    }

    SetSameNode(pParse,&ppTest[3],ppTemp);
    SetSameNode(pParse,&ppTest[4],ppTemp);
    rc = SubstProp(pParse,ppTest[3]->pRight,ppTest[4]);
   // rc = SubstProp(pParse,ppTest[3]->pLeft,ppTest[4]);
    log_a("rc %d",rc);

    PrintSubstAst(pParse,ppTest[3]);
    PrintSubstAst(pParse,ppTest[4]);
    ClearSubstFlag(pParse,ppTest[3]);
    ClearSubstFlag(pParse,ppTest[4]);
    PrintSubstAst(pParse,ppTest[3]);
    PrintSubstAst(pParse,ppTest[4]);
    for(i=0; i<pParse->all_num; i++)
    {
        FreeAstTree(pParse,&ppTest[i],ppTemp);
    }
}


void  SubstMpTest(AstParse *pParse,Vector *pSet)
{
    int i;
    TokenInfo *ppTemp[100];//存在递归时的共享变量
    TokenInfo *pR;//
    TokenInfo *pDemo;
    TokenInfo *pLeft;
    TokenInfo **ppTest = pSet->data;
    TokenInfo **ppDbSet;
    Vector *pVec;

    pParse->ppTemp = ppTemp;
    for(i=0; i<3; i++)
    {
        SetSameNode(pParse,&ppTest[i],ppTemp);
        printf("num:%d\n",i+1);
        PrintAst(pParse,ppTest[i]);
        BeginSqliteWrite(pParse);
        WriteAxiomStr(pParse,ppTest[i]);
        EndSqliteWrite(pParse);
    }

    BeginSqliteWrite(pParse);
    for(i=3;i<pParse->axiom_num;i++)
    {
        NewMemPool(pParse,1000000);
        log_a("old i %d",i+1);
        if(i==18){
            printf("ss:%d\n",i+1);
        }
        PrintAst(pParse,ppTest[i]);
        if(ppTest[i]->isDeduction){
            int n = 0;
            n = SetSameNode(pParse,&ppTest[i],ppTemp);
            printf("n same %d\n",n);
        }
        pR = PropMpSeq(pParse,ppTest,ppTest[i]);
        if(pR!=NULL){
            pR = CopyAstTree(pParse,pR,0);
            FreePropSeq(pParse,ppTest[i],ppTemp);
            FreeAstTree(pParse,&ppTest[i],ppTemp);
            ppTest[i] = pR;
        }
        log_a("new i %d",i+1);
        PrintAst(pParse,ppTest[i]);
        SetSameNode(pParse,&ppTest[i],ppTemp);
        FreeMemPool(pParse);
    }
    for(; i<pParse->all_num; i++)
    {
        NewMemPool(pParse,1000000);
        printf("num:%d\n",i+1);
        PrintAst(pParse,ppTest[i]);
        SetSameNode(pParse,&ppTest[i],ppTemp);
        pR = PropGenSeq(pParse,ppTest,ppTest[i]);
        printf("seq %d\n",i+1);
        PrintAst(pParse,pR);

        //把序列重新生成定理
        printf("prop %d\n",i+1);
        pParse->usePool = 1;
        SetSameNode(pParse,&pR,ppTemp);
        assert(!pParse->usePool);
        PropMpSeq(pParse,ppTest,pR);
        PrintAst(pParse,pR->pTheorem);
        if(pR->pTheorem!=NULL){
            FreePropSeq(pParse,pR,ppTemp);
        }
        FreeMemPool(pParse);
    }
    EndSqliteWrite(pParse);

#if 1
    pVec = InitTheoremSet(pParse);
    SqliteReadTable(pParse,pParse->pDb->db,"TheoremSet",pVec);
    ppDbSet = pVec->data;
    for(i=0; i<pVec->n; i++)
    {
        NewMemPool(pParse,1000000);
        printf("row:%d\n",i+1);
        PrintAst(pParse,ppDbSet[i]);
        SetSameNode(pParse,&ppDbSet[i],ppTemp);
        pR = PropGenSeq(pParse,ppDbSet,ppDbSet[i]);
        printf("seq %d\n",i+1);
        PrintAst(pParse,pR);
        FreeMemPool(pParse);
    }
   // pDemo = CopyAstTree(pParse,ppDbSet[56],0);
    pDemo = CopyAstTree(pParse,ppDbSet[31],0);
    PrintAst(pParse,pDemo);
    for(i=0; i<pVec->n; i++)
    {
        NewMemPool(pParse,1000000);
        if(gDebug.mpLeftDebug){
            printf("mp left:%d\n",i+1);
            PrintAst(pParse,ppDbSet[i]);
        }
        pR = SubstMpLeft(pParse,pDemo,ppDbSet[i]);
        if(pR!=NULL){

            pLeft = PropGenSeq(pParse,NULL,pR);
            if(pLeft!=NULL){
                printf("get left %d\n",i+1);
                //PrintAst(pParse,pR);
                PropMpSeq(pParse,ppTest,pLeft);
                PrintAst(pParse,pLeft->pTheorem);
                if(pLeft->pTheorem!=NULL){
                    FreePropSeq(pParse,pLeft,ppTemp);
                }
                printf("--------\n");
                PrintAst(pParse,ppDbSet[i]);
                log_a("");
            }

            FreeAstTree(pParse,&pR,ppTemp);
        }
        FreeMemPool(pParse);
    }
    FreeAstTree(pParse,&pDemo,ppTemp);
#endif
    FreeVector(pParse,pSet);
    FreeVector(pParse,pVec);
    pParse->ppTemp = NULL;

}
