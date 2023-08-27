EXTERN lpRemain:QWORD
EXTERN ogAddress:QWORD

.data
storageFixed BYTE 0

.code
hkStorageFix PROC
	cmp storageFixed,0
	jnz SkipFixStorage

	mov storageFixed,1
	jmp Exit

	SkipFixStorage:
		call ogAddress

	Exit:
		jmp lpRemain
hkStorageFix ENDP
END