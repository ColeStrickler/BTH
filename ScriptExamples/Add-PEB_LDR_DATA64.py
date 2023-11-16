import mgr


id = mgr.NewStructure("PEB_LDR_DATA64")

mgr.AddStructMember(id, "Length", 4, 2)
mgr.AddStructMember(id, "Initialized", 1, 7)
mgr.AddStructMember(id, "SsHandle", 8, 3)
mgr.AddStructMember(id, "InLoadOrderModuleList_flink",8, 3)
mgr.AddStructMember(id, "InLoadOrderModuleList_blink",8, 3)
mgr.AddStructMember(id, "InMemoryOrderModuleList_flink",8, 3)
mgr.AddStructMember(id, "InMemoryOrderModuleList_blink",8, 3)
mgr.AddStructMember(id, "InInitializationOrderModuleList_flink",8, 3)
mgr.AddStructMember(id, "InInitializationOrderModuleList_blink",8, 3)
mgr.AddStructMember(id, "EntryInProgress", 8, 3)
mgr.AddStructMember(id, "ShutdownInProgress",1, 7)
mgr.AddStructMember(id, "ShutdownThreadId",8, 3)