#include "StdAfx.h"
#include "Class.h"
#include "ClassHeap.h"
#include "ObjectHeap.h"

Class::Class(void):
	m_pClassHeap(NULL),
	m_pByteCode(NULL),
	m_nByteCodeLength(0),
	m_nObjectFieldsCount(0)
{
}

Class::~Class(void)
{
	if(m_pByteCode) delete m_pByteCode;
}

BOOL Class::LoadClassFromFile(CString classFilepath)
{
    u1 *p;
    size_t lenRead, len;

	CFile file;
	CFileException e;

	ASSERT(PathFileExists(classFilepath));
	if(!PathFileExists(classFilepath)) return FALSE;

	BOOL bRet=file.Open(classFilepath, CFile::modeRead, &e);
	ASSERT(bRet);
	if(!bRet) return FALSE;

	len=(size_t)file.GetLength();	    
    
    m_nByteCodeLength = len;

    p = (u1 *) new u1[len+2];
    if (p != NULL)
    {
		lenRead=file.Read(p, (UINT)len);
        if (len != lenRead)
        {
            delete p;
            p = NULL;
        }
    }
    else
	{
		file.Close();
		return FALSE;
	}
	file.Close();

    if (p)
	{
		p[len] = 0;
		SetByteCode(p);
	}

	return (p != NULL);
}

void Class::SetByteCode(void* pByteCode)
{
	if(m_pByteCode) delete m_pByteCode;
	m_pByteCode=pByteCode;

	if(m_pByteCode) ParseClass();
}

BOOL Class::ParseClass(void)

	if(m_pByteCode==NULL || m_nByteCodeLength < sizeof(ClassFileFormat)+20)
		return FALSE;
	char *p=(char *)m_pByteCode;
	magic = getu4(p); p+=4;
	ASSERT(magic == 0xCAFEBABE);
	minor_version=getu2(p); p+=2;
    major_version=getu2(p); p+=2;
    constant_pool_count=getu2(p); p+=2;

	if(constant_pool_count>0)
		ParseConstantPool(p);

    access_flags=getu2(p); p+=2;
    this_class=getu2(p); p+=2;
    super_class=getu2(p); p+=2;
    interfaces_count=getu2(p); p+=2;

	if(interfaces_count>0)
		ParseInterfaces(p);

	fields_count=getu2(p); p+=2;

	if(fields_count > 0)
		ParseFields(p);

	methods_count = getu2(p);p+=2;

	if(methods_count > 0)
	{
		ParseMethods(p);
	}

	attributes_count = getu2(p);p+=2;


	if(attributes_count > 0)
		ParseAttributes(p);

	return 0;
}

BOOL Class::ParseAttributes(char* &p)
{
	attributes = new attribute_info*[attributes_count];

	if(methods == NULL) return FALSE;
	

	for(int i=0;i<attributes_count;i++)
	{
		attributes[i]=(attribute_info*)p;
		u2 name_index=getu2(p);p+=2;
		u4 len=getu4(p);p+=4;
		p+=len;

	}

	return TRUE;
}

BOOL Class::ParseMethods(char* &p)
{
	methods = new method_info[methods_count];

	if(methods == NULL) return FALSE;
	
	for(int i=0;i<methods_count;i++)
	{
		methods[i].pMethodInfoBase=(method_info_in*)p;
		methods[i].access_flags= getu2(p);	p+=2;
		methods[i].name_index = getu2(p); p+=2; 
		methods[i].descriptor_index= getu2(p); p+=2;
		methods[i].attributes_count=getu2(p); p+=2;
		
		CString strName, strDesc;
		GetStringFromConstPool(methods[i].name_index, strName);
		GetStringFromConstPool(methods[i].descriptor_index, strDesc);


		methods[i].pCode_attr=NULL;
		if(methods[i].attributes_count>0)
		{
			for(int a=0;a<methods[i].attributes_count;a++)
			{
				u2 name_index=getu2(p); p+=2;
				
				u4 len=getu4(p);p+=4;
				p+=len;
			}

			methods[i].pCode_attr = new Code_attribute;
			ParseMethodCodeAttribute(i, methods[i].pCode_attr);
		}		
	}

	return TRUE;
}

BOOL Class::ParseConstantPool(char* &p)
{	
	constant_pool = new cp_info*[constant_pool_count-1];

	if(constant_pool == NULL) return FALSE;
	

	for(int i=1;i<constant_pool_count;i++)
	{
		constant_pool[i]=(cp_info*)p;
		
		int size = GetConstantPoolSize(p);
		p+= size;


		if(constant_pool[i]->tag == CONSTANT_Long || constant_pool[i]->tag == CONSTANT_Double)
		{
			constant_pool[i+1]=NULL;
			i++;
		}
	}

	return TRUE;
}

BOOL Class::ParseFields(char* &p)
{
	fields = new field_info_ex[fields_count];

	if(fields == NULL) return FALSE;
	

	for(int i=0;i<fields_count;i++)
	{
		fields[i].pFieldInfoBase = (field_info*)p;

		fields[i].access_flags= getu2(p); p+=2;
		fields[i].name_index= getu2(p);p+=2;
		fields[i].descriptor_index= getu2(p);p+=2;
		fields[i].attributes_count=getu2(p); p+=2;
		
		if(fields[i].attributes_count>0)
		{
			for(int a=0;a<fields[i].attributes_count;a++)
			{
				u2 name_index=getu2(p); p+=2;
				u4 len=getu4(p);p+=4;
				p+=len;
			}
		}
		
	}

	return TRUE;

}
BOOL Class::ParseInterfaces(char* &p)
{	
	interfaces = new u2[interfaces_count];
	
	for(int i=0;i<interfaces_count;i++)
	{
		interfaces[i] = getu2(p); p+=2;
	}

	return TRUE;
}

int Class::GetConstantPoolSize(char* p)
{
	cp_info* cp=(cp_info *)p;

	switch(cp->tag)
	{
	case CONSTANT_Class:
		return	3;
	case CONSTANT_Fieldref:
		return 5;
	case CONSTANT_Methodref:
		return 5;
	case CONSTANT_InterfaceMethodref:
		return 5;
	case CONSTANT_String:
		return 3;
	case CONSTANT_Integer:
		return 5;
	case CONSTANT_Float:
		return 5;
	case CONSTANT_Long:
		return 9;
	case CONSTANT_Double:
		return 9;
	case CONSTANT_NameAndType:
		return 5;
	case CONSTANT_Utf8:
		return 3+getu2(p+1);
	default:
		ASSERT(FALSE);
		break;
	}

	return 0;
}

BOOL Class::GetConstantPool(u2 nIndex, cp_info& const_pool)
{
	if(nIndex>constant_pool_count-1) return FALSE;

	char *cpool= (char *)constant_pool[nIndex];


	const_pool.tag = cpool[0]; cpool++;
	if(CONSTANT_Utf8 == const_pool.tag)
	{
		int len = getu2(cpool);	
		cpool+=2;
		const_pool.info = new u1[len+1];
		memset(const_pool.info, 0, len+1);
		memcpy(const_pool.info , cpool, len);
	}
	if(CONSTANT_Class == const_pool.tag)
	{
		const_pool.info = new u1[2];
		memcpy(const_pool.info , cpool, 2);
	}


	return 0;
}

BOOL Class::GetStringFromConstPool(int nIndex, CString& strValue)
{

	if(nIndex<1 || nIndex >= constant_pool_count)
	{
		return FALSE;
	}
	if(constant_pool[nIndex]->tag != CONSTANT_Utf8)
		return FALSE;

	u1 *p =(u1 *)constant_pool[nIndex];

	short length=getu2(&p[1]);
	char *buffer= new char[length+1];
	buffer[length]=0;
	memcpy(buffer, &p[3], length);
	strValue += buffer;
	delete buffer;
	return TRUE;
}

CString Class::GetName(void)
{
	CString retVal=_T("");
	if(constant_pool[this_class]->tag != CONSTANT_Class)
		return retVal;
	char *bc=(char*)constant_pool[this_class];
	u2 name_index= getu2(&bc[1]);
	GetStringFromConstPool(name_index, retVal);
	return retVal;
}

CString Class::GetSuperClassName(void)
{
	CString retVal=_T("");
	if(super_class<1) return retVal;

	if(constant_pool[super_class]->tag != CONSTANT_Class)
		return retVal;
	char *bc=(char*)constant_pool[super_class];
	u2 name_index= getu2(&bc[1]);
	GetStringFromConstPool(name_index, retVal);
	return retVal;
}

BOOL Class::ParseMethodCodeAttribute(int nMethodIndex, Code_attribute* pCode_attr)
{
	if(methods==NULL || nMethodIndex > methods_count) return FALSE;
	char *pMi, *bc;
	pMi= bc = (char *)methods[nMethodIndex].pMethodInfoBase;
	bc+=6;
	int nAttributes = getu2(bc); bc+=2;
	
	if(nAttributes>0)
	{
		for(int a=0;a<nAttributes;a++)
		{
			u2 name_index=getu2(bc); bc+=2;		
			CString strAttributeName;
			GetStringFromConstPool(name_index, strAttributeName);

			if(!strAttributeName.CompareNoCase(_T("Code")))
			{
				char *ca=bc;
				pCode_attr->attribute_name_index=name_index;
				pCode_attr->attribute_length=getu4(ca); ca+=4;
				pCode_attr->max_stack=getu2(ca); ca+=2;
				pCode_attr->max_locals=getu2(ca); ca+=2;
				pCode_attr->code_length=getu4(ca); ca+=4;
				if(pCode_attr->code_length>0)
				{
					pCode_attr->code = new u1[pCode_attr->code_length];
					memcpy(pCode_attr->code,ca, pCode_attr->code_length);


				}
				else
				{
					pCode_attr->code=NULL;
				}
				ca+=pCode_attr->code_length;
				pCode_attr->exception_table_length = getu2(ca);ca+=2;

				if(pCode_attr->exception_table_length > 0)
				{
					pCode_attr->exception_table = new Exception_table[pCode_attr->exception_table_length];
					for(int ext= 0; ext<pCode_attr->exception_table_length; ext++)
					{
						pCode_attr->exception_table[ext].start_pc = getu2(ca); ca+=2;
						pCode_attr->exception_table[ext].end_pc = getu2(ca); ca+=2;
						pCode_attr->exception_table[ext].handler_pc = getu2(ca); ca+=2;
						pCode_attr->exception_table[ext].catch_type = getu2(ca); ca+=2;
					}
				}				
			}

			u4 len=getu4(bc);bc+=4;
			bc+=len;
		}
	}		


	return 0;
}

int Class::GetMethodIndex(CString strMethodName, CString strMethodDesc,Class* &pClass)
{
	if(methods == NULL) return FALSE;
	
	Class* pCurClass=this;
	while(pCurClass)
	{
		for(int i=0;i<pCurClass->methods_count;i++)
		{
			CString name, desc;

			pCurClass->GetStringFromConstPool(pCurClass->methods[i].name_index, name);
			if(name.Compare(strMethodName)) continue;

			pCurClass->GetStringFromConstPool(pCurClass->methods[i].descriptor_index, desc);

			if(!desc.Compare(strMethodDesc))
			{
				if(pClass)
					pClass = pCurClass;

				return i;
			}
		}

		if(pClass != NULL)
		{
			pCurClass = pCurClass->GetSuperClass();
		}
		else
		{
			break;
		}
	}

	return -1;
}

int Class::GetFieldIndex(CString strName, CString& strDesc)
{
	if(fields == NULL) return FALSE;
	
	for(int i=0;i<fields_count;i++)
	{
		CString name, desc;
		GetStringFromConstPool(fields[i].name_index, name);
		if(name.Compare(strName)) continue;

		GetStringFromConstPool(fields[i].descriptor_index, desc);

		strDesc =desc;
		return i;
	}

	return -1;
}

u4 Class::GetObjectSize(void)
{
	u4 size= fields_count * sizeof(Variable);
	CString superClass=GetSuperClassName();
	Class *pSuperClass = m_pClassHeap->GetClass(superClass);
	u4 superObjSize=0;
	if(pSuperClass)
		superObjSize=pSuperClass->GetObjectSize();

	size+= superObjSize;
	return size;
}

u4 Class::GetObjectFieldCount(void)
{
	u4 count= fields_count;
	CString superClass=GetSuperClassName();
	Class *pSuperClass = m_pClassHeap->GetClass(superClass);
	u4 superObjFieldCount=0;
	if(pSuperClass)
		superObjFieldCount=pSuperClass->GetObjectFieldCount();

	count+= superObjFieldCount;
	return count;
}

Class* Class::GetSuperClass(void)
{
	CString superClass=GetSuperClassName();
	Class *pSuperClass = m_pClassHeap->GetClass(superClass);

	return pSuperClass;
}

BOOL Class::CreateObject(u2 index, ObjectHeap *pObjectHeap, Object& object)
{
	char *cp=(char*)this->constant_pool[index];
	ASSERT(cp[0] == CONSTANT_Class);
	ASSERT(pObjectHeap);
	if(cp[0] != CONSTANT_Class)
		return FALSE;

	u2 name_index=getu2(&cp[1]);
	CString strClassName;
	if(!this->GetStringFromConstPool(name_index, strClassName))
		return FALSE;
	
	DbgPrint("Creating new object of class [%ws]\n", strClassName);

	Class *pNewClass=this->m_pClassHeap->GetClass(strClassName);
	if(pNewClass == NULL) return FALSE;
	object=pObjectHeap->CreateObject(pNewClass);
	return TRUE;
}

BOOL Class::CreateObjectArray(u2 index, u4 count, ObjectHeap *pObjectHeap, Object& object)
{
	char *cp=(char*)this->constant_pool[index];
	ASSERT(cp[0] == CONSTANT_Class);
	ASSERT(pObjectHeap);
	if(cp[0] != CONSTANT_Class)
		return FALSE;

	u2 name_index=getu2(&cp[1]);
	CString strClassName;
	if(!this->GetStringFromConstPool(name_index, strClassName))
		return FALSE;
	Class *pNewClass=this->m_pClassHeap->GetClass(strClassName);
	if(pNewClass == NULL) return FALSE;

	return pObjectHeap->CreateObjectArray(pNewClass, count, object);
}

