import mgr


id = mgr.NewStructure("PEB_LDR_DATA32")

mgr.AddStructMember(id, "Length", 4, 2)
mgr.AddStructMember(id, "Initialized", 1, 7)
mgr.AddStructMember(id, "SsHandle", 4, 2)
mgr.AddStructMember(id, "InLoadOrderModuleList_flink",4, 2)
mgr.AddStructMember(id, "InLoadOrderModuleList_blink",4, 2)
mgr.AddStructMember(id, "InMemoryOrderModuleList_flink",4, 2)
mgr.AddStructMember(id, "InMemoryOrderModuleList_blink",4, 2)
mgr.AddStructMember(id, "InInitializationOrderModuleList_flink",4, 2)
mgr.AddStructMember(id, "InInitializationOrderModuleList_blink",4, 2)
mgr.AddStructMember(id, "EntryInProgress", 4, 2)
mgr.AddStructMember(id, "ShutdownInProgress",1, 7)
mgr.AddStructMember(id, "ShutdownThreadId",4, 2)