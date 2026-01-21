# 0.3.0 (12/7/2025)
  - Implemented backups fully
  - Implemented zip compression
  - Added ATTRIBUTION.txt
  - NBT changes:
    - `data` (`CompoundTag`)
	  - `backups` (`ListTag<CompoundTag>`)
        - `<specification>` (`CompoundTag`): Create
		  - `backupTime` (`LongTag`): Create
		  - `compressionFormat` (`IntTag`): Create
		  - `relativePath` (`StringTag`): Create
	  - `backupConfig` (`CompoundTag`): Create
	    - `backupFormat` (`ByteTag`): Create
	    - `backupNameFormat` (`StringTag`): Create
		- `destinationFolder` (`StringTag`): Create
# 0.2.0 (12/03/2025)
  - Added options to backup and view backups, but still not implemented
  - Implemented some GUI functionality with Dear ImGui
  - Bug fixes
  - NBT changes: none
# 0.1.0 (12/02/2025)
  - Basic functionality
  - CLI implemented with some stubs
  - NBT changes:
	- `data` (`CompoundTag`): Create
	  - `backups` (`ListTag<CompoundTag>`): Create
      - `version` (`IntTag`): Create
# 0.0.1 (12/01/2025)
  - Initial release
