(module
  (type (;0;) (func (param i32) (result i32)))
  (type (;1;) (func (param i32 i32) (result i32)))
  (type (;2;) (func (result i32)))
  (type (;3;) (func (param i32 i32 i32) (result i32)))
  (type (;4;) (func (param i32 i32 i32 i32 i32 i32 i32 i32 i32 i32) (result i32)))
  (type (;5;) (func (param i32 i32 i32 i32 i32) (result i32)))
  (type (;6;) (func (param i32 i32 i32 i32 i32 i32) (result i32)))
  (type (;7;) (func (param i32 i32 i32 i32) (result i32)))
  (import "env" "memcmp" (func (;0;) (type 3)))
  (import "env" "memcpy" (func (;1;) (type 3)))
  (import "env" "memmove" (func (;2;) (type 3)))
  (import "env" "memset" (func (;3;) (type 3)))
  (import "env" "alloc_memory" (func (;4;) (type 0)))
  (import "env" "fopen" (func (;5;) (type 1)))
  (import "env" "fread" (func (;6;) (type 7)))
  (import "env" "fclose" (func (;7;) (type 0)))
  (import "env" "fwrite" (func (;8;) (type 7)))
  (func $__lzo_ptr_linear (type 0) (param i32) (result i32)
    local.get 0)
  (func $__lzo_align_gap (type 1) (param i32 i32) (result i32)
    (local i32)
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 1
      i32.const 2
      i32.lt_u
      br_if 0 (;@1;)
      local.get 1
      local.get 1
      i32.const -1
      i32.add
      i32.and
      br_if 0 (;@1;)
      local.get 1
      local.get 0
      i32.add
      i32.const -1
      i32.add
      i32.const 0
      local.get 1
      i32.sub
      i32.and
      local.get 0
      i32.sub
      local.set 2
    end
    local.get 2)
  (func $lzo_copyright (type 2) (result i32)
    i32.const 0)
  (func $lzo_version (type 2) (result i32)
    i32.const 8352)
  (func $lzo_version_string (type 2) (result i32)
    i32.const 5)
  (func $lzo_version_date (type 2) (result i32)
    i32.const 10)
  (func $_lzo_version_string (type 2) (result i32)
    i32.const 5)
  (func $_lzo_version_date (type 2) (result i32)
    i32.const 10)
  (func $lzo_adler32 (type 3) (param i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32)
    block  ;; label = @1
      local.get 1
      br_if 0 (;@1;)
      i32.const 1
      return
    end
    local.get 0
    i32.const 16
    i32.shr_u
    local.set 3
    local.get 0
    i32.const 65535
    i32.and
    local.set 4
    block  ;; label = @1
      local.get 2
      i32.eqz
      br_if 0 (;@1;)
      loop  ;; label = @2
        local.get 2
        local.get 2
        i32.const 5552
        local.get 2
        i32.const 5552
        i32.lt_u
        select
        local.tee 5
        i32.sub
        local.set 6
        block  ;; label = @3
          block  ;; label = @4
            local.get 5
            i32.const 16
            i32.lt_u
            br_if 0 (;@4;)
            local.get 5
            i32.const -16
            i32.add
            local.tee 7
            i32.const -16
            i32.and
            local.set 2
            local.get 1
            local.set 0
            loop  ;; label = @5
              local.get 4
              local.get 0
              i32.load8_u
              i32.add
              local.tee 4
              local.get 3
              i32.add
              local.get 4
              local.get 0
              i32.const 1
              i32.add
              i32.load8_u
              i32.add
              local.tee 3
              i32.add
              local.get 3
              local.get 0
              i32.const 2
              i32.add
              i32.load8_u
              i32.add
              local.tee 3
              i32.add
              local.get 3
              local.get 0
              i32.const 3
              i32.add
              i32.load8_u
              i32.add
              local.tee 3
              i32.add
              local.get 3
              local.get 0
              i32.const 4
              i32.add
              i32.load8_u
              i32.add
              local.tee 3
              i32.add
              local.get 3
              local.get 0
              i32.const 5
              i32.add
              i32.load8_u
              i32.add
              local.tee 3
              i32.add
              local.get 3
              local.get 0
              i32.const 6
              i32.add
              i32.load8_u
              i32.add
              local.tee 3
              i32.add
              local.get 3
              local.get 0
              i32.const 7
              i32.add
              i32.load8_u
              i32.add
              local.tee 3
              i32.add
              local.get 3
              local.get 0
              i32.const 8
              i32.add
              i32.load8_u
              i32.add
              local.tee 3
              i32.add
              local.get 3
              local.get 0
              i32.const 9
              i32.add
              i32.load8_u
              i32.add
              local.tee 3
              i32.add
              local.get 3
              local.get 0
              i32.const 10
              i32.add
              i32.load8_u
              i32.add
              local.tee 3
              i32.add
              local.get 3
              local.get 0
              i32.const 11
              i32.add
              i32.load8_u
              i32.add
              local.tee 3
              i32.add
              local.get 3
              local.get 0
              i32.const 12
              i32.add
              i32.load8_u
              i32.add
              local.tee 3
              i32.add
              local.get 3
              local.get 0
              i32.const 13
              i32.add
              i32.load8_u
              i32.add
              local.tee 3
              i32.add
              local.get 3
              local.get 0
              i32.const 14
              i32.add
              i32.load8_u
              i32.add
              local.tee 3
              i32.add
              local.get 3
              local.get 0
              i32.const 15
              i32.add
              i32.load8_u
              i32.add
              local.tee 4
              i32.add
              local.set 3
              local.get 0
              i32.const 16
              i32.add
              local.set 0
              local.get 5
              i32.const -16
              i32.add
              local.tee 5
              i32.const 15
              i32.gt_u
              br_if 0 (;@5;)
            end
            local.get 1
            local.get 2
            i32.add
            i32.const 16
            i32.add
            local.set 1
            local.get 7
            local.get 2
            i32.sub
            local.tee 2
            i32.eqz
            br_if 1 (;@3;)
          end
          local.get 1
          local.set 0
          local.get 2
          local.set 5
          loop  ;; label = @4
            local.get 4
            local.get 0
            i32.load8_u
            i32.add
            local.tee 4
            local.get 3
            i32.add
            local.set 3
            local.get 0
            i32.const 1
            i32.add
            local.set 0
            local.get 5
            i32.const -1
            i32.add
            local.tee 5
            br_if 0 (;@4;)
          end
          local.get 1
          local.get 2
          i32.add
          local.set 1
        end
        local.get 3
        i32.const 65521
        i32.rem_u
        local.set 3
        local.get 4
        i32.const 65521
        i32.rem_u
        local.set 4
        local.get 6
        local.set 2
        local.get 6
        br_if 0 (;@2;)
      end
    end
    local.get 3
    i32.const 16
    i32.shl
    local.get 4
    i32.or)
  (func $lzo_memcmp (type 3) (param i32 i32 i32) (result i32)
    local.get 0
    local.get 1
    local.get 2
    call 0)
  (func $lzo_memcpy (type 3) (param i32 i32 i32) (result i32)
    local.get 0
    local.get 1
    local.get 2
    call 1)
  (func $lzo_memmove (type 3) (param i32 i32 i32) (result i32)
    local.get 0
    local.get 1
    local.get 2
    call 2)
  (func $lzo_memset (type 3) (param i32 i32 i32) (result i32)
    local.get 0
    local.get 1
    local.get 2
    call 3)
  (func $_lzo_config_check (type 2) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    i64.const 0
    i64.store offset=24
    i32.const 0
    call $u2p
    local.tee 0
    i32.load8_u
    local.set 1
    i32.const 0
    i64.const 128
    i64.store offset=24
    local.get 0
    i32.load
    local.set 2
    i32.const 0
    i64.const 33554433
    i64.store offset=24
    i32.const 1
    call $u2p
    local.tee 0
    i32.load16_u align=1
    local.set 3
    i32.const 0
    i32.const 128
    i32.store8 offset=25
    local.get 0
    i32.load16_u align=1
    local.set 4
    i32.const 0
    i32.const 129
    i32.store8 offset=26
    local.get 0
    i32.load16_u align=1
    local.set 5
    i32.const 0
    i64.const 4398046511107
    i64.store offset=24
    local.get 0
    i32.const 1
    i32.add
    local.tee 6
    i32.load8_u
    local.set 7
    local.get 0
    i32.load8_u
    local.set 8
    i32.const 0
    i32.const 128
    i32.store8 offset=25
    local.get 6
    i32.load8_u
    local.set 9
    local.get 0
    i32.load8_u
    local.set 10
    local.get 0
    i32.load8_u offset=3
    local.set 11
    local.get 0
    i32.load8_u offset=2
    local.set 12
    i32.const 0
    i32.const 131
    i32.store8 offset=28
    i32.const 0
    i32.const 33409
    i32.store16 offset=26
    i32.const -1
    local.set 6
    block  ;; label = @1
      local.get 1
      br_if 0 (;@1;)
      local.get 2
      i32.const 128
      i32.ne
      br_if 0 (;@1;)
      local.get 3
      i32.const 65535
      i32.and
      br_if 0 (;@1;)
      local.get 4
      i32.const 65535
      i32.and
      i32.const 128
      i32.ne
      br_if 0 (;@1;)
      local.get 5
      i32.const 65535
      i32.and
      i32.const 33152
      i32.ne
      br_if 0 (;@1;)
      local.get 11
      i32.const 24
      i32.shl
      local.get 12
      i32.const 16
      i32.shl
      i32.or
      local.tee 1
      local.get 8
      i32.or
      local.get 7
      i32.const 8
      i32.shl
      i32.or
      br_if 0 (;@1;)
      local.get 1
      local.get 10
      i32.or
      local.get 9
      i32.const 8
      i32.shl
      i32.or
      i32.const 128
      i32.ne
      br_if 0 (;@1;)
      local.get 0
      i32.load align=1
      i32.const -2088599168
      i32.ne
      br_if 0 (;@1;)
      i32.const 31
      local.set 0
      i32.const 1
      local.set 6
      block  ;; label = @2
        loop  ;; label = @3
          local.get 0
          local.get 6
          i32.clz
          i32.ne
          local.tee 1
          br_if 1 (;@2;)
          local.get 0
          i32.const -1
          i32.add
          local.set 0
          local.get 6
          i32.const 1
          i32.shl
          local.tee 6
          br_if 0 (;@3;)
        end
      end
      i32.const -1
      local.set 6
      local.get 1
      br_if 0 (;@1;)
      i32.const 0
      local.set 0
      i32.const 1
      local.set 6
      block  ;; label = @2
        loop  ;; label = @3
          local.get 0
          local.get 6
          i32.ctz
          i32.ne
          local.tee 1
          br_if 1 (;@2;)
          local.get 0
          i32.const 1
          i32.add
          local.set 0
          local.get 6
          i32.const 1
          i32.shl
          local.tee 6
          br_if 0 (;@3;)
        end
      end
      i32.const -1
      i32.const 0
      local.get 1
      select
      local.set 6
    end
    local.get 6)
  (func $u2p (type 0) (param i32) (result i32)
    local.get 0
    i32.const 24
    i32.add)
  (func $__lzo_init_v2 (type 4) (param i32 i32 i32 i32 i32 i32 i32 i32 i32 i32) (result i32)
    (local i32)
    i32.const -1
    local.set 10
    block  ;; label = @1
      local.get 0
      i32.eqz
      br_if 0 (;@1;)
      local.get 1
      i32.const 1
      i32.add
      local.tee 0
      i32.const 3
      i32.gt_u
      br_if 0 (;@1;)
      block  ;; label = @2
        local.get 0
        br_table 0 (;@2;) 1 (;@1;) 1 (;@1;) 0 (;@2;) 0 (;@2;)
      end
      local.get 2
      i32.const 1
      i32.add
      local.tee 0
      i32.const 5
      i32.gt_u
      br_if 0 (;@1;)
      block  ;; label = @2
        local.get 0
        br_table 0 (;@2;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 0 (;@2;) 0 (;@2;)
      end
      local.get 3
      i32.const 1
      i32.add
      local.tee 0
      i32.const 5
      i32.gt_u
      br_if 0 (;@1;)
      block  ;; label = @2
        local.get 0
        br_table 0 (;@2;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 0 (;@2;) 0 (;@2;)
      end
      local.get 4
      i32.const 1
      i32.add
      local.tee 0
      i32.const 5
      i32.gt_u
      br_if 0 (;@1;)
      block  ;; label = @2
        local.get 0
        br_table 0 (;@2;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 0 (;@2;) 0 (;@2;)
      end
      local.get 5
      i32.const 1
      i32.add
      local.tee 0
      i32.const 5
      i32.gt_u
      br_if 0 (;@1;)
      block  ;; label = @2
        local.get 0
        br_table 0 (;@2;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 0 (;@2;) 0 (;@2;)
      end
      local.get 6
      i32.const 1
      i32.add
      local.tee 0
      i32.const 5
      i32.gt_u
      br_if 0 (;@1;)
      block  ;; label = @2
        local.get 0
        br_table 0 (;@2;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 0 (;@2;) 0 (;@2;)
      end
      local.get 7
      i32.const 1
      i32.add
      local.tee 0
      i32.const 5
      i32.gt_u
      br_if 0 (;@1;)
      block  ;; label = @2
        local.get 0
        br_table 0 (;@2;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 0 (;@2;) 0 (;@2;)
      end
      local.get 8
      i32.const 1
      i32.add
      local.tee 0
      i32.const 5
      i32.gt_u
      br_if 0 (;@1;)
      block  ;; label = @2
        local.get 0
        br_table 0 (;@2;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 0 (;@2;) 0 (;@2;)
      end
      block  ;; label = @2
        local.get 9
        i32.const 24
        i32.eq
        br_if 0 (;@2;)
        local.get 9
        i32.const -1
        i32.ne
        br_if 1 (;@1;)
      end
      call $_lzo_config_check
      local.set 10
    end
    local.get 10)
  (func $lzo1x_1_compress (type 5) (param i32 i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 5
    local.get 1
    local.set 6
    local.get 2
    local.set 7
    block  ;; label = @1
      local.get 1
      i32.const 21
      i32.lt_u
      br_if 0 (;@1;)
      local.get 0
      local.set 8
      local.get 2
      local.set 7
      local.get 1
      local.set 6
      i32.const 0
      local.set 5
      loop  ;; label = @2
        local.get 6
        i32.const 49152
        local.get 6
        i32.const 49152
        i32.lt_u
        select
        local.tee 9
        local.get 8
        i32.add
        local.tee 10
        local.get 9
        local.get 5
        i32.add
        i32.const 5
        i32.shr_u
        i32.add
        local.tee 11
        local.get 10
        i32.le_u
        br_if 1 (;@1;)
        local.get 10
        local.get 11
        i32.ge_u
        br_if 1 (;@1;)
        local.get 8
        local.get 9
        local.get 7
        local.get 3
        local.get 5
        local.get 4
        i32.const 0
        i32.const 32768
        call 3
        call $lzo1x_1_compress_core
        local.set 5
        local.get 7
        local.get 3
        i32.load
        i32.add
        local.set 7
        local.get 10
        local.set 8
        local.get 6
        local.get 9
        i32.sub
        local.tee 6
        i32.const 20
        i32.gt_u
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      local.get 6
      local.get 5
      i32.add
      local.tee 8
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 1
      i32.add
      local.tee 5
      local.get 8
      i32.sub
      local.set 6
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              local.get 7
              local.get 2
              i32.ne
              br_if 0 (;@5;)
              local.get 8
              i32.const 238
              i32.gt_u
              br_if 0 (;@5;)
              local.get 2
              local.get 8
              i32.const 17
              i32.add
              i32.store8
              local.get 2
              i32.const 1
              i32.add
              local.set 11
              br 1 (;@4;)
            end
            block  ;; label = @5
              local.get 8
              i32.const 3
              i32.gt_u
              br_if 0 (;@5;)
              local.get 7
              i32.const -2
              i32.add
              local.tee 10
              local.get 10
              i32.load8_u
              local.get 8
              i32.or
              i32.store8
              local.get 8
              local.set 10
              local.get 7
              local.set 11
              br 2 (;@3;)
            end
            block  ;; label = @5
              local.get 8
              i32.const 18
              i32.gt_u
              br_if 0 (;@5;)
              local.get 7
              local.get 8
              i32.const -3
              i32.add
              i32.store8
              local.get 7
              i32.const 1
              i32.add
              local.set 11
              br 1 (;@4;)
            end
            local.get 7
            i32.const 0
            i32.store8
            local.get 7
            i32.const 1
            i32.add
            local.set 10
            block  ;; label = @5
              local.get 8
              i32.const -18
              i32.add
              local.tee 9
              i32.const 256
              i32.lt_u
              br_if 0 (;@5;)
              loop  ;; label = @6
                local.get 10
                i32.const 0
                i32.store8
                local.get 10
                i32.const 1
                i32.add
                local.set 10
                local.get 9
                i32.const -255
                i32.add
                local.tee 9
                i32.const 255
                i32.gt_u
                br_if 0 (;@6;)
              end
              local.get 10
              i32.const -1
              i32.add
              local.set 7
            end
            local.get 10
            local.get 9
            i32.store8
            local.get 7
            i32.const 2
            i32.add
            local.set 11
          end
          local.get 11
          local.set 7
          local.get 8
          local.set 10
          block  ;; label = @4
            local.get 8
            i32.const 8
            i32.lt_u
            br_if 0 (;@4;)
            i32.const 0
            local.get 8
            i32.sub
            local.set 10
            local.get 8
            local.set 9
            local.get 11
            local.set 7
            loop  ;; label = @5
              local.get 7
              local.get 5
              local.get 10
              i32.add
              local.tee 6
              i32.load8_u
              i32.store8
              local.get 7
              i32.const 1
              i32.add
              local.get 6
              i32.const 1
              i32.add
              i32.load8_u
              i32.store8
              local.get 7
              i32.const 2
              i32.add
              local.get 6
              i32.const 2
              i32.add
              i32.load8_u
              i32.store8
              local.get 7
              i32.const 3
              i32.add
              local.get 6
              i32.const 3
              i32.add
              i32.load8_u
              i32.store8
              local.get 7
              i32.const 4
              i32.add
              local.get 6
              i32.const 4
              i32.add
              i32.load8_u
              i32.store8
              local.get 7
              i32.const 5
              i32.add
              local.get 6
              i32.const 5
              i32.add
              i32.load8_u
              i32.store8
              local.get 7
              i32.const 6
              i32.add
              local.get 6
              i32.const 6
              i32.add
              i32.load8_u
              i32.store8
              local.get 7
              i32.const 7
              i32.add
              local.get 6
              i32.const 7
              i32.add
              i32.load8_u
              i32.store8
              local.get 7
              i32.const 8
              i32.add
              local.set 7
              local.get 10
              i32.const 8
              i32.add
              local.set 10
              local.get 9
              i32.const -8
              i32.add
              local.tee 9
              i32.const 7
              i32.gt_u
              br_if 0 (;@5;)
            end
            local.get 5
            local.get 9
            i32.sub
            local.set 6
            local.get 8
            i32.const 7
            i32.and
            local.set 10
          end
          block  ;; label = @4
            local.get 10
            i32.const 4
            i32.lt_u
            br_if 0 (;@4;)
            local.get 7
            local.get 6
            i32.load8_u
            i32.store8
            local.get 7
            local.get 6
            i32.load8_u offset=1
            i32.store8 offset=1
            local.get 7
            local.get 6
            i32.load8_u offset=2
            i32.store8 offset=2
            local.get 7
            local.get 6
            i32.load8_u offset=3
            i32.store8 offset=3
            local.get 10
            i32.const -4
            i32.add
            local.set 10
            local.get 6
            i32.const 4
            i32.add
            local.set 6
            local.get 7
            i32.const 4
            i32.add
            local.set 7
          end
          local.get 10
          i32.eqz
          br_if 1 (;@2;)
        end
        loop  ;; label = @3
          local.get 7
          local.get 6
          i32.load8_u
          i32.store8
          local.get 7
          i32.const 1
          i32.add
          local.set 7
          local.get 6
          i32.const 1
          i32.add
          local.set 6
          local.get 10
          i32.const -1
          i32.add
          local.tee 10
          br_if 0 (;@3;)
        end
      end
      local.get 11
      local.get 8
      i32.add
      local.set 7
    end
    local.get 7
    i32.const 0
    i32.store8 offset=2
    local.get 7
    i32.const 17
    i32.store16 align=1
    local.get 3
    local.get 7
    local.get 2
    i32.sub
    i32.const 3
    i32.add
    i32.store
    i32.const 0)
  (func $lzo1x_1_compress_core (type 6) (param i32 i32 i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32)
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 0
        i32.const 4
        local.get 4
        i32.sub
        local.tee 6
        local.get 6
        i32.const 4
        i32.gt_u
        select
        i32.add
        i32.const 1
        i32.add
        local.tee 6
        local.get 0
        local.get 1
        i32.add
        local.tee 7
        i32.const -20
        i32.add
        local.tee 8
        i32.lt_u
        br_if 0 (;@2;)
        local.get 0
        local.set 6
        local.get 2
        local.set 1
        br 1 (;@1;)
      end
      local.get 2
      local.set 1
      local.get 0
      local.set 9
      loop  ;; label = @2
        local.get 6
        local.set 10
        block  ;; label = @3
          loop  ;; label = @4
            local.get 5
            local.get 10
            i32.load align=1
            local.tee 6
            i32.const 405029533
            i32.mul
            i32.const 17
            i32.shr_u
            i32.const 32766
            i32.and
            i32.add
            local.tee 11
            i32.load16_u
            local.set 12
            local.get 11
            local.get 10
            local.get 0
            i32.sub
            i32.store16
            local.get 6
            local.get 0
            local.get 12
            i32.add
            local.tee 12
            i32.load align=1
            i32.eq
            br_if 1 (;@3;)
            local.get 10
            local.get 9
            i32.sub
            i32.const 5
            i32.shr_s
            local.get 10
            i32.add
            i32.const 1
            i32.add
            local.tee 10
            local.get 8
            i32.lt_u
            br_if 0 (;@4;)
          end
          local.get 9
          local.set 6
          br 2 (;@1;)
        end
        block  ;; label = @3
          local.get 10
          local.get 9
          local.get 4
          i32.sub
          local.tee 4
          i32.sub
          local.tee 6
          i32.eqz
          br_if 0 (;@3;)
          block  ;; label = @4
            local.get 6
            i32.const 3
            i32.gt_u
            br_if 0 (;@4;)
            local.get 1
            i32.const -2
            i32.add
            local.tee 9
            local.get 9
            i32.load8_u
            local.get 6
            i32.or
            i32.store8
            local.get 4
            local.get 10
            i32.sub
            local.set 6
            loop  ;; label = @5
              local.get 1
              local.get 4
              i32.load8_u
              i32.store8
              local.get 1
              i32.const 1
              i32.add
              local.set 1
              local.get 4
              i32.const 1
              i32.add
              local.set 4
              local.get 6
              i32.const 1
              i32.add
              local.tee 9
              local.get 6
              i32.lt_u
              local.set 11
              local.get 9
              local.set 6
              local.get 11
              i32.eqz
              br_if 0 (;@5;)
              br 2 (;@3;)
            end
          end
          block  ;; label = @4
            block  ;; label = @5
              local.get 6
              i32.const 18
              i32.gt_u
              br_if 0 (;@5;)
              local.get 1
              local.get 6
              i32.const -3
              i32.add
              i32.store8
              local.get 1
              i32.const 1
              i32.add
              local.set 1
              br 1 (;@4;)
            end
            local.get 1
            i32.const 0
            i32.store8
            local.get 1
            i32.const 1
            i32.add
            local.set 9
            block  ;; label = @5
              local.get 6
              i32.const -18
              i32.add
              local.tee 6
              i32.const 256
              i32.lt_u
              br_if 0 (;@5;)
              loop  ;; label = @6
                local.get 9
                i32.const 0
                i32.store8
                local.get 9
                i32.const 1
                i32.add
                local.set 9
                local.get 6
                i32.const -255
                i32.add
                local.tee 6
                i32.const 255
                i32.gt_u
                br_if 0 (;@6;)
              end
              local.get 9
              i32.const -1
              i32.add
              local.set 1
            end
            local.get 9
            local.get 6
            i32.store8
            local.get 1
            i32.const 2
            i32.add
            local.set 1
          end
          local.get 4
          local.get 10
          i32.sub
          local.set 6
          loop  ;; label = @4
            local.get 1
            local.get 4
            i32.load8_u
            i32.store8
            local.get 1
            i32.const 1
            i32.add
            local.set 1
            local.get 4
            i32.const 1
            i32.add
            local.set 4
            local.get 6
            i32.const 1
            i32.add
            local.tee 9
            local.get 6
            i32.ge_u
            local.set 11
            local.get 9
            local.set 6
            local.get 11
            br_if 0 (;@4;)
          end
        end
        i32.const 4
        local.set 4
        block  ;; label = @3
          local.get 10
          i32.load8_u offset=4
          local.get 12
          i32.load8_u offset=4
          i32.ne
          br_if 0 (;@3;)
          i32.const 4
          local.set 11
          i32.const 6
          local.set 6
          block  ;; label = @4
            block  ;; label = @5
              loop  ;; label = @6
                local.get 10
                local.get 6
                local.tee 4
                i32.add
                local.tee 6
                i32.const -1
                i32.add
                i32.load8_u
                local.get 12
                local.get 4
                i32.add
                local.tee 9
                i32.const -1
                i32.add
                i32.load8_u
                i32.ne
                br_if 1 (;@5;)
                local.get 6
                i32.load8_u
                local.get 9
                i32.load8_u
                i32.ne
                br_if 3 (;@3;)
                local.get 6
                i32.const 1
                i32.add
                i32.load8_u
                local.get 9
                i32.const 1
                i32.add
                i32.load8_u
                i32.ne
                br_if 2 (;@4;)
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          local.get 6
                          i32.const 2
                          i32.add
                          i32.load8_u
                          local.get 9
                          i32.const 2
                          i32.add
                          i32.load8_u
                          i32.ne
                          br_if 0 (;@11;)
                          local.get 6
                          i32.const 3
                          i32.add
                          i32.load8_u
                          local.get 9
                          i32.const 3
                          i32.add
                          i32.load8_u
                          i32.ne
                          br_if 1 (;@10;)
                          local.get 6
                          i32.const 4
                          i32.add
                          i32.load8_u
                          local.get 9
                          i32.const 4
                          i32.add
                          i32.load8_u
                          i32.ne
                          br_if 2 (;@9;)
                          local.get 6
                          i32.const 5
                          i32.add
                          i32.load8_u
                          local.get 9
                          i32.const 5
                          i32.add
                          i32.load8_u
                          i32.ne
                          br_if 3 (;@8;)
                          local.get 11
                          i32.const 8
                          i32.add
                          local.set 11
                          local.get 6
                          i32.const 6
                          i32.add
                          local.tee 13
                          local.get 8
                          i32.lt_u
                          br_if 4 (;@7;)
                          local.get 11
                          local.set 4
                          br 8 (;@3;)
                        end
                        local.get 4
                        i32.const 2
                        i32.add
                        local.set 4
                        br 7 (;@3;)
                      end
                      local.get 4
                      i32.const 3
                      i32.add
                      local.set 4
                      br 6 (;@3;)
                    end
                    local.get 4
                    i32.const 4
                    i32.add
                    local.set 4
                    br 5 (;@3;)
                  end
                  local.get 4
                  i32.const 5
                  i32.add
                  local.set 4
                  br 4 (;@3;)
                end
                local.get 4
                i32.const 8
                i32.add
                local.set 6
                local.get 13
                i32.load8_u
                local.get 9
                i32.const 6
                i32.add
                i32.load8_u
                i32.eq
                br_if 0 (;@6;)
              end
              local.get 4
              i32.const 6
              i32.add
              local.set 4
              br 2 (;@3;)
            end
            local.get 4
            i32.const -1
            i32.add
            local.set 4
            br 1 (;@3;)
          end
          local.get 4
          i32.const 1
          i32.add
          local.set 4
        end
        local.get 10
        local.get 4
        i32.add
        local.set 6
        block  ;; label = @3
          block  ;; label = @4
            local.get 10
            local.get 12
            i32.sub
            local.tee 9
            i32.const 2048
            i32.gt_u
            br_if 0 (;@4;)
            local.get 4
            i32.const 8
            i32.gt_u
            br_if 0 (;@4;)
            local.get 1
            local.get 4
            i32.const 5
            i32.shl
            local.get 9
            i32.const -1
            i32.add
            local.tee 4
            i32.const 2
            i32.shl
            i32.const 28
            i32.and
            i32.or
            i32.const 224
            i32.add
            i32.store8
            local.get 4
            i32.const 3
            i32.shr_u
            local.set 4
            br 1 (;@3;)
          end
          block  ;; label = @4
            local.get 9
            i32.const 16384
            i32.gt_u
            br_if 0 (;@4;)
            local.get 9
            i32.const -1
            i32.add
            local.set 9
            block  ;; label = @5
              block  ;; label = @6
                local.get 4
                i32.const 33
                i32.gt_u
                br_if 0 (;@6;)
                local.get 1
                local.get 4
                i32.const -2
                i32.add
                i32.const 32
                i32.or
                i32.store8
                local.get 1
                i32.const 1
                i32.add
                local.set 1
                br 1 (;@5;)
              end
              local.get 1
              i32.const 32
              i32.store8
              local.get 1
              i32.const 1
              i32.add
              local.set 11
              block  ;; label = @6
                local.get 4
                i32.const -33
                i32.add
                local.tee 4
                i32.const 256
                i32.lt_u
                br_if 0 (;@6;)
                loop  ;; label = @7
                  local.get 11
                  i32.const 0
                  i32.store8
                  local.get 11
                  i32.const 1
                  i32.add
                  local.set 11
                  local.get 4
                  i32.const -255
                  i32.add
                  local.tee 4
                  i32.const 255
                  i32.gt_u
                  br_if 0 (;@7;)
                end
                local.get 11
                i32.const -1
                i32.add
                local.set 1
              end
              local.get 11
              local.get 4
              i32.store8
              local.get 1
              i32.const 2
              i32.add
              local.set 1
            end
            local.get 1
            local.get 9
            i32.const 2
            i32.shl
            i32.store8
            local.get 9
            i32.const 6
            i32.shr_u
            local.set 4
            br 1 (;@3;)
          end
          local.get 9
          i32.const -16384
          i32.add
          local.set 9
          block  ;; label = @4
            block  ;; label = @5
              local.get 4
              i32.const 9
              i32.gt_u
              br_if 0 (;@5;)
              local.get 1
              local.get 4
              i32.const 254
              i32.add
              local.get 9
              i32.const 11
              i32.shr_u
              i32.const 8
              i32.and
              i32.or
              i32.const 16
              i32.or
              i32.store8
              local.get 1
              i32.const 1
              i32.add
              local.set 1
              br 1 (;@4;)
            end
            local.get 1
            local.get 9
            i32.const 11
            i32.shr_u
            i32.const 8
            i32.and
            i32.const 16
            i32.or
            i32.store8
            local.get 1
            i32.const 1
            i32.add
            local.set 11
            block  ;; label = @5
              local.get 4
              i32.const -9
              i32.add
              local.tee 4
              i32.const 256
              i32.lt_u
              br_if 0 (;@5;)
              loop  ;; label = @6
                local.get 11
                i32.const 0
                i32.store8
                local.get 11
                i32.const 1
                i32.add
                local.set 11
                local.get 4
                i32.const -255
                i32.add
                local.tee 4
                i32.const 255
                i32.gt_u
                br_if 0 (;@6;)
              end
              local.get 11
              i32.const -1
              i32.add
              local.set 1
            end
            local.get 11
            local.get 4
            i32.store8
            local.get 1
            i32.const 2
            i32.add
            local.set 1
          end
          local.get 1
          local.get 9
          i32.const 2
          i32.shl
          i32.store8
          local.get 9
          i32.const 6
          i32.shr_u
          local.set 4
        end
        local.get 1
        local.get 4
        i32.store8 offset=1
        local.get 1
        i32.const 2
        i32.add
        local.set 1
        i32.const 0
        local.set 4
        local.get 6
        local.set 9
        local.get 6
        local.get 8
        i32.lt_u
        br_if 0 (;@2;)
      end
    end
    local.get 3
    local.get 1
    local.get 2
    i32.sub
    i32.store
    local.get 7
    local.get 4
    local.get 6
    i32.sub
    i32.add)
  (func $lzo1x_decompress (type 5) (param i32 i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32)
    local.get 3
    i32.const 0
    i32.store
    block  ;; label = @1
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              local.get 0
              i32.load8_u
              local.tee 5
              i32.const 18
              i32.ge_u
              br_if 0 (;@5;)
              local.get 0
              local.set 6
              local.get 2
              local.set 7
              br 1 (;@4;)
            end
            local.get 0
            i32.const 1
            i32.add
            local.set 6
            block  ;; label = @5
              local.get 5
              i32.const -17
              i32.add
              local.tee 8
              i32.const 4
              i32.ge_u
              br_if 0 (;@5;)
              local.get 2
              local.set 7
              br 3 (;@2;)
            end
            local.get 2
            local.set 9
            loop  ;; label = @5
              local.get 9
              local.get 6
              i32.load8_u
              i32.store8
              local.get 9
              i32.const 1
              i32.add
              local.set 9
              local.get 6
              i32.const 1
              i32.add
              local.set 6
              local.get 8
              i32.const -1
              i32.add
              local.tee 8
              br_if 0 (;@5;)
              br 2 (;@3;)
            end
          end
          i32.const 0
          local.set 10
          br 2 (;@1;)
        end
        i32.const 1
        local.set 10
        br 1 (;@1;)
      end
      i32.const 2
      local.set 10
    end
    loop (result i32)  ;; label = @1
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  local.get 10
                  br_table 0 (;@7;) 1 (;@6;) 2 (;@5;) 2 (;@5;)
                end
                local.get 6
                i32.const 1
                i32.add
                local.set 11
                block  ;; label = @7
                  local.get 5
                  i32.const 255
                  i32.and
                  local.tee 10
                  i32.const 15
                  i32.le_u
                  br_if 0 (;@7;)
                  local.get 7
                  local.set 9
                  br 3 (;@4;)
                end
                block  ;; label = @7
                  local.get 5
                  i32.const 255
                  i32.and
                  br_if 0 (;@7;)
                  i32.const 0
                  local.set 10
                  block  ;; label = @8
                    block  ;; label = @9
                      local.get 11
                      i32.load8_u
                      local.tee 12
                      i32.eqz
                      br_if 0 (;@9;)
                      local.get 6
                      local.set 9
                      br 1 (;@8;)
                    end
                    i32.const 0
                    local.set 10
                    loop  ;; label = @9
                      local.get 10
                      i32.const 255
                      i32.add
                      local.set 10
                      local.get 6
                      i32.const 2
                      i32.add
                      local.set 12
                      local.get 6
                      i32.const 1
                      i32.add
                      local.tee 9
                      local.set 6
                      local.get 12
                      i32.load8_u
                      local.tee 12
                      i32.eqz
                      br_if 0 (;@9;)
                    end
                  end
                  local.get 9
                  i32.const 2
                  i32.add
                  local.set 11
                  local.get 10
                  local.get 12
                  i32.add
                  i32.const 15
                  i32.add
                  local.set 10
                end
                local.get 7
                local.get 11
                i32.load8_u
                i32.store8
                local.get 7
                local.get 11
                i32.load8_u offset=1
                i32.store8 offset=1
                local.get 7
                local.get 11
                i32.load8_u offset=2
                i32.store8 offset=2
                local.get 7
                i32.const 3
                i32.add
                local.set 9
                local.get 11
                i32.const 3
                i32.add
                local.set 6
                loop  ;; label = @7
                  local.get 9
                  local.get 6
                  i32.load8_u
                  i32.store8
                  local.get 9
                  i32.const 1
                  i32.add
                  local.set 9
                  local.get 6
                  i32.const 1
                  i32.add
                  local.set 6
                  local.get 10
                  i32.const -1
                  i32.add
                  local.tee 10
                  br_if 0 (;@7;)
                end
                i32.const 1
                local.set 10
                br 5 (;@1;)
              end
              block  ;; label = @6
                local.get 6
                i32.load8_u
                local.tee 10
                i32.const 15
                i32.le_u
                br_if 0 (;@6;)
                local.get 6
                i32.const 1
                i32.add
                local.set 11
                br 2 (;@4;)
              end
              local.get 9
              local.get 9
              local.get 10
              i32.const 2
              i32.shr_u
              i32.sub
              local.get 6
              i32.load8_u offset=1
              i32.const 2
              i32.shl
              i32.sub
              i32.const -2049
              i32.add
              local.tee 7
              i32.load8_u
              i32.store8
              local.get 9
              local.get 7
              i32.load8_u offset=1
              i32.store8 offset=1
              local.get 7
              i32.const 1
              i32.add
              local.set 10
              local.get 9
              i32.const 3
              i32.add
              local.set 7
              local.get 9
              i32.const 2
              i32.add
              local.set 12
              local.get 6
              i32.const 2
              i32.add
              local.set 6
              br 2 (;@3;)
            end
            local.get 7
            local.get 6
            i32.load8_u
            i32.store8
            block  ;; label = @5
              block  ;; label = @6
                local.get 8
                i32.const 2
                i32.ge_u
                br_if 0 (;@6;)
                local.get 7
                i32.const 1
                i32.add
                local.set 9
                local.get 6
                i32.const 1
                i32.add
                local.set 7
                br 1 (;@5;)
              end
              local.get 7
              local.get 6
              i32.load8_u offset=1
              i32.store8 offset=1
              block  ;; label = @6
                local.get 8
                i32.const 2
                i32.ne
                br_if 0 (;@6;)
                local.get 7
                i32.const 2
                i32.add
                local.set 9
                local.get 6
                i32.const 2
                i32.add
                local.set 7
                br 1 (;@5;)
              end
              local.get 7
              local.get 6
              i32.load8_u offset=2
              i32.store8 offset=2
              local.get 7
              i32.const 3
              i32.add
              local.set 9
              local.get 6
              i32.const 3
              i32.add
              local.set 7
            end
            local.get 7
            i32.const 1
            i32.add
            local.set 11
            local.get 7
            i32.load8_u
            local.set 10
          end
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  local.get 10
                  i32.const 64
                  i32.lt_u
                  br_if 0 (;@7;)
                  local.get 11
                  i32.load8_u
                  i32.const 3
                  i32.shl
                  i32.const -1
                  i32.xor
                  local.get 9
                  local.get 10
                  i32.const 2
                  i32.shr_u
                  i32.const 7
                  i32.and
                  i32.sub
                  i32.add
                  local.set 8
                  local.get 11
                  i32.const 1
                  i32.add
                  local.set 6
                  local.get 10
                  i32.const 5
                  i32.shr_u
                  i32.const -1
                  i32.add
                  local.set 12
                  br 1 (;@6;)
                end
                block  ;; label = @7
                  local.get 10
                  i32.const 32
                  i32.lt_u
                  br_if 0 (;@7;)
                  block  ;; label = @8
                    local.get 10
                    i32.const 31
                    i32.and
                    local.tee 12
                    br_if 0 (;@8;)
                    i32.const 0
                    local.set 7
                    block  ;; label = @9
                      block  ;; label = @10
                        local.get 11
                        i32.load8_u
                        local.tee 10
                        i32.eqz
                        br_if 0 (;@10;)
                        local.get 11
                        local.set 12
                        br 1 (;@9;)
                      end
                      i32.const 0
                      local.set 7
                      loop  ;; label = @10
                        local.get 7
                        i32.const 255
                        i32.add
                        local.set 7
                        local.get 11
                        i32.load8_u offset=1
                        local.set 10
                        local.get 11
                        i32.const 1
                        i32.add
                        local.tee 12
                        local.set 11
                        local.get 10
                        i32.eqz
                        br_if 0 (;@10;)
                      end
                    end
                    local.get 12
                    i32.const 1
                    i32.add
                    local.set 11
                    local.get 7
                    local.get 10
                    i32.add
                    i32.const 31
                    i32.add
                    local.set 12
                  end
                  local.get 11
                  i32.load8_u offset=1
                  i32.const 6
                  i32.shl
                  local.get 11
                  i32.load8_u
                  i32.const 2
                  i32.shr_u
                  i32.or
                  i32.const -1
                  i32.xor
                  local.get 9
                  i32.add
                  local.set 8
                  local.get 11
                  i32.const 2
                  i32.add
                  local.set 6
                  br 1 (;@6;)
                end
                local.get 10
                i32.const 16
                i32.lt_u
                br_if 2 (;@4;)
                local.get 9
                local.get 10
                i32.const 11
                i32.shl
                i32.const 16384
                i32.and
                i32.sub
                local.set 8
                block  ;; label = @7
                  local.get 10
                  i32.const 7
                  i32.and
                  local.tee 12
                  br_if 0 (;@7;)
                  i32.const 0
                  local.set 7
                  block  ;; label = @8
                    block  ;; label = @9
                      local.get 11
                      i32.load8_u
                      local.tee 10
                      i32.eqz
                      br_if 0 (;@9;)
                      local.get 11
                      local.set 12
                      br 1 (;@8;)
                    end
                    i32.const 0
                    local.set 7
                    loop  ;; label = @9
                      local.get 7
                      i32.const 255
                      i32.add
                      local.set 7
                      local.get 11
                      i32.load8_u offset=1
                      local.set 10
                      local.get 11
                      i32.const 1
                      i32.add
                      local.tee 12
                      local.set 11
                      local.get 10
                      i32.eqz
                      br_if 0 (;@9;)
                    end
                  end
                  local.get 12
                  i32.const 1
                  i32.add
                  local.set 11
                  local.get 7
                  local.get 10
                  i32.add
                  i32.const 7
                  i32.add
                  local.set 12
                end
                local.get 0
                local.get 1
                i32.add
                local.set 10
                local.get 11
                i32.const 2
                i32.add
                local.set 6
                local.get 8
                local.get 11
                i32.load8_u offset=1
                i32.const 6
                i32.shl
                local.get 11
                i32.load8_u
                i32.const 2
                i32.shr_u
                i32.or
                i32.sub
                local.tee 7
                local.get 9
                i32.eq
                br_if 1 (;@5;)
                local.get 7
                i32.const -16384
                i32.add
                local.set 8
              end
              local.get 9
              local.get 8
              i32.load8_u
              i32.store8
              local.get 9
              local.get 8
              i32.load8_u offset=1
              i32.store8 offset=1
              local.get 9
              i32.const 2
              i32.add
              local.set 7
              local.get 8
              i32.const 2
              i32.add
              local.set 10
              loop  ;; label = @6
                local.get 7
                local.get 10
                i32.load8_u
                i32.store8
                local.get 7
                i32.const 1
                i32.add
                local.set 7
                local.get 10
                i32.const 1
                i32.add
                local.set 10
                local.get 12
                i32.const -1
                i32.add
                local.tee 12
                br_if 0 (;@6;)
                br 4 (;@2;)
              end
            end
            local.get 3
            local.get 9
            local.get 2
            i32.sub
            i32.store
            i32.const 0
            i32.const -8
            i32.const -4
            local.get 6
            local.get 10
            i32.lt_u
            select
            local.get 6
            local.get 10
            i32.eq
            select
            return
          end
          local.get 9
          local.get 11
          i32.load8_u
          i32.const 2
          i32.shl
          i32.const -1
          i32.xor
          local.get 9
          local.get 10
          i32.const 2
          i32.shr_u
          i32.sub
          i32.add
          local.tee 10
          i32.load8_u
          i32.store8
          local.get 9
          i32.const 2
          i32.add
          local.set 7
          local.get 9
          i32.const 1
          i32.add
          local.set 12
          local.get 11
          i32.const 1
          i32.add
          local.set 6
        end
        local.get 12
        local.get 10
        i32.load8_u offset=1
        i32.store8
      end
      block  ;; label = @2
        local.get 6
        i32.const -2
        i32.add
        i32.load8_u
        i32.const 3
        i32.and
        local.tee 8
        br_if 0 (;@2;)
        local.get 6
        i32.load8_u
        local.set 5
        i32.const 0
        local.set 10
        br 1 (;@1;)
      end
      i32.const 2
      local.set 10
      br 0 (;@1;)
    end)
  (func $lzo1x_decompress_safe (type 5) (param i32 i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32)
    local.get 3
    i32.load
    local.set 5
    local.get 3
    i32.const 0
    i32.store
    local.get 2
    local.set 6
    block  ;; label = @1
      local.get 1
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 1
      i32.add
      local.set 7
      local.get 2
      local.get 5
      i32.add
      local.set 8
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            local.get 0
            i32.load8_u
            local.tee 9
            i32.const 18
            i32.ge_u
            br_if 0 (;@4;)
            local.get 2
            local.set 6
            i32.const 0
            local.set 1
            br 1 (;@3;)
          end
          local.get 0
          i32.const 1
          i32.add
          local.set 0
          block  ;; label = @4
            block  ;; label = @5
              local.get 9
              i32.const -17
              i32.add
              local.tee 10
              i32.const 4
              i32.ge_u
              br_if 0 (;@5;)
              local.get 2
              local.set 6
              br 1 (;@4;)
            end
            local.get 2
            local.set 6
            local.get 5
            local.get 10
            i32.lt_u
            br_if 2 (;@2;)
            local.get 2
            local.set 6
            local.get 1
            i32.const -1
            i32.add
            local.get 9
            i32.const -14
            i32.add
            i32.lt_u
            br_if 3 (;@1;)
            local.get 2
            local.set 11
            loop  ;; label = @5
              local.get 11
              local.get 0
              i32.load8_u
              i32.store8
              local.get 11
              i32.const 1
              i32.add
              local.set 11
              local.get 0
              i32.const 1
              i32.add
              local.set 0
              local.get 10
              i32.const -1
              i32.add
              local.tee 10
              br_if 0 (;@5;)
            end
            i32.const 1
            local.set 1
            br 1 (;@3;)
          end
          i32.const 2
          local.set 1
        end
        loop  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          block  ;; label = @12
                            block  ;; label = @13
                              block  ;; label = @14
                                local.get 1
                                br_table 0 (;@14;) 1 (;@13;) 2 (;@12;) 2 (;@12;)
                              end
                              local.get 7
                              local.get 0
                              i32.sub
                              i32.const 3
                              i32.lt_u
                              br_if 12 (;@1;)
                              local.get 0
                              i32.const 1
                              i32.add
                              local.set 5
                              block  ;; label = @14
                                local.get 0
                                i32.load8_u
                                local.tee 1
                                i32.const 15
                                i32.le_u
                                br_if 0 (;@14;)
                                local.get 6
                                local.set 11
                                br 3 (;@11;)
                              end
                              block  ;; label = @14
                                local.get 1
                                br_if 0 (;@14;)
                                local.get 0
                                i32.const 2
                                i32.add
                                local.set 5
                                i32.const 15
                                local.set 1
                                block  ;; label = @15
                                  loop  ;; label = @16
                                    local.get 5
                                    i32.const -1
                                    i32.add
                                    i32.load8_u
                                    local.tee 11
                                    br_if 1 (;@15;)
                                    local.get 7
                                    local.get 5
                                    i32.eq
                                    br_if 15 (;@1;)
                                    local.get 1
                                    i32.const -15
                                    i32.add
                                    local.set 11
                                    local.get 1
                                    i32.const 255
                                    i32.add
                                    local.set 1
                                    local.get 5
                                    i32.const 1
                                    i32.add
                                    local.set 5
                                    local.get 11
                                    i32.const -766
                                    i32.le_u
                                    br_if 0 (;@16;)
                                    br 15 (;@1;)
                                  end
                                end
                                local.get 11
                                local.get 1
                                i32.add
                                local.set 1
                              end
                              local.get 8
                              local.get 6
                              i32.sub
                              local.get 1
                              i32.const 3
                              i32.add
                              i32.lt_u
                              br_if 11 (;@2;)
                              local.get 7
                              local.get 5
                              i32.sub
                              local.get 1
                              i32.const 6
                              i32.add
                              i32.lt_u
                              br_if 12 (;@1;)
                              local.get 6
                              local.get 5
                              i32.load8_u
                              i32.store8
                              local.get 6
                              local.get 5
                              i32.load8_u offset=1
                              i32.store8 offset=1
                              local.get 6
                              local.get 5
                              i32.load8_u offset=2
                              i32.store8 offset=2
                              local.get 6
                              i32.const 3
                              i32.add
                              local.set 11
                              local.get 5
                              i32.const 3
                              i32.add
                              local.set 0
                              loop  ;; label = @14
                                local.get 11
                                local.get 0
                                i32.load8_u
                                i32.store8
                                local.get 11
                                i32.const 1
                                i32.add
                                local.set 11
                                local.get 0
                                i32.const 1
                                i32.add
                                local.set 0
                                local.get 1
                                i32.const -1
                                i32.add
                                local.tee 1
                                br_if 0 (;@14;)
                              end
                              i32.const 1
                              local.set 1
                              br 10 (;@3;)
                            end
                            block  ;; label = @13
                              local.get 0
                              i32.load8_u
                              local.tee 1
                              i32.const 15
                              i32.le_u
                              br_if 0 (;@13;)
                              local.get 0
                              i32.const 1
                              i32.add
                              local.set 5
                              br 2 (;@11;)
                            end
                            local.get 11
                            local.get 1
                            i32.const 2
                            i32.shr_u
                            i32.sub
                            local.get 0
                            i32.load8_u offset=1
                            i32.const 2
                            i32.shl
                            i32.sub
                            i32.const -2049
                            i32.add
                            local.tee 1
                            local.get 2
                            i32.lt_u
                            br_if 7 (;@5;)
                            local.get 1
                            local.get 11
                            i32.ge_u
                            br_if 7 (;@5;)
                            local.get 8
                            local.get 11
                            i32.sub
                            i32.const 3
                            i32.lt_u
                            br_if 2 (;@10;)
                            local.get 0
                            i32.const 2
                            i32.add
                            local.set 0
                            local.get 11
                            local.get 1
                            i32.load8_u
                            i32.store8
                            local.get 11
                            local.get 1
                            i32.load8_u offset=1
                            i32.store8 offset=1
                            local.get 11
                            i32.const 3
                            i32.add
                            local.set 6
                            local.get 11
                            i32.const 2
                            i32.add
                            local.set 10
                            local.get 1
                            i32.const 1
                            i32.add
                            local.set 1
                            br 4 (;@8;)
                          end
                          local.get 8
                          local.get 6
                          i32.sub
                          local.get 10
                          i32.lt_u
                          br_if 9 (;@2;)
                          local.get 7
                          local.get 0
                          i32.sub
                          local.get 10
                          i32.const 3
                          i32.add
                          i32.lt_u
                          br_if 10 (;@1;)
                          local.get 6
                          local.get 0
                          i32.load8_u
                          i32.store8
                          block  ;; label = @12
                            block  ;; label = @13
                              local.get 10
                              i32.const 2
                              i32.ge_u
                              br_if 0 (;@13;)
                              local.get 6
                              i32.const 1
                              i32.add
                              local.set 11
                              local.get 0
                              i32.const 1
                              i32.add
                              local.set 6
                              br 1 (;@12;)
                            end
                            local.get 6
                            local.get 0
                            i32.load8_u offset=1
                            i32.store8 offset=1
                            block  ;; label = @13
                              local.get 10
                              i32.const 2
                              i32.ne
                              br_if 0 (;@13;)
                              local.get 6
                              i32.const 2
                              i32.add
                              local.set 11
                              local.get 0
                              i32.const 2
                              i32.add
                              local.set 6
                              br 1 (;@12;)
                            end
                            local.get 6
                            local.get 0
                            i32.load8_u offset=2
                            i32.store8 offset=2
                            local.get 6
                            i32.const 3
                            i32.add
                            local.set 11
                            local.get 0
                            i32.const 3
                            i32.add
                            local.set 6
                          end
                          local.get 6
                          i32.const 1
                          i32.add
                          local.set 5
                          local.get 6
                          i32.load8_u
                          local.set 1
                        end
                        block  ;; label = @11
                          block  ;; label = @12
                            block  ;; label = @13
                              local.get 1
                              i32.const 64
                              i32.lt_u
                              br_if 0 (;@13;)
                              local.get 5
                              i32.load8_u
                              i32.const 3
                              i32.shl
                              i32.const -1
                              i32.xor
                              local.get 11
                              local.get 1
                              i32.const 2
                              i32.shr_u
                              i32.const 7
                              i32.and
                              i32.sub
                              i32.add
                              local.tee 9
                              local.get 2
                              i32.lt_u
                              br_if 8 (;@5;)
                              local.get 9
                              local.get 11
                              i32.ge_u
                              br_if 8 (;@5;)
                              local.get 8
                              local.get 11
                              i32.sub
                              local.get 1
                              i32.const 5
                              i32.shr_u
                              local.tee 6
                              i32.le_u
                              br_if 3 (;@10;)
                              local.get 5
                              i32.const 1
                              i32.add
                              local.set 0
                              local.get 6
                              i32.const -1
                              i32.add
                              local.set 10
                              br 1 (;@12;)
                            end
                            block  ;; label = @13
                              block  ;; label = @14
                                block  ;; label = @15
                                  block  ;; label = @16
                                    block  ;; label = @17
                                      local.get 1
                                      i32.const 32
                                      i32.lt_u
                                      br_if 0 (;@17;)
                                      i32.const 31
                                      local.set 6
                                      block  ;; label = @18
                                        local.get 1
                                        i32.const 31
                                        i32.and
                                        local.tee 10
                                        br_if 0 (;@18;)
                                        block  ;; label = @19
                                          loop  ;; label = @20
                                            local.get 5
                                            i32.load8_u
                                            local.tee 1
                                            br_if 1 (;@19;)
                                            local.get 6
                                            i32.const -31
                                            i32.add
                                            i32.const -766
                                            i32.gt_u
                                            br_if 10 (;@10;)
                                            local.get 6
                                            i32.const 255
                                            i32.add
                                            local.set 6
                                            local.get 7
                                            local.get 5
                                            i32.const 1
                                            i32.add
                                            local.tee 5
                                            i32.ne
                                            br_if 0 (;@20;)
                                            br 4 (;@16;)
                                          end
                                        end
                                        local.get 7
                                        local.get 5
                                        i32.const 1
                                        i32.add
                                        local.tee 5
                                        i32.sub
                                        i32.const 2
                                        i32.lt_u
                                        br_if 2 (;@16;)
                                        local.get 1
                                        local.get 6
                                        i32.add
                                        local.set 10
                                      end
                                      local.get 5
                                      i32.load8_u offset=1
                                      i32.const 6
                                      i32.shl
                                      local.get 5
                                      i32.load8_u
                                      i32.const 2
                                      i32.shr_u
                                      i32.or
                                      i32.const -1
                                      i32.xor
                                      local.get 11
                                      i32.add
                                      local.set 9
                                      local.get 5
                                      i32.const 2
                                      i32.add
                                      local.set 0
                                      br 4 (;@13;)
                                    end
                                    local.get 1
                                    i32.const 16
                                    i32.lt_u
                                    br_if 5 (;@11;)
                                    i32.const 7
                                    local.set 6
                                    local.get 1
                                    i32.const 7
                                    i32.and
                                    local.tee 10
                                    br_if 2 (;@14;)
                                    block  ;; label = @17
                                      loop  ;; label = @18
                                        local.get 5
                                        i32.load8_u
                                        local.tee 10
                                        br_if 1 (;@17;)
                                        local.get 6
                                        i32.const -7
                                        i32.add
                                        i32.const -766
                                        i32.gt_u
                                        br_if 8 (;@10;)
                                        local.get 6
                                        i32.const 255
                                        i32.add
                                        local.set 6
                                        local.get 7
                                        local.get 5
                                        i32.const 1
                                        i32.add
                                        local.tee 5
                                        i32.ne
                                        br_if 0 (;@18;)
                                        br 2 (;@16;)
                                      end
                                    end
                                    local.get 7
                                    local.get 5
                                    i32.const 1
                                    i32.add
                                    local.tee 5
                                    i32.sub
                                    i32.const 2
                                    i32.ge_u
                                    br_if 1 (;@15;)
                                  end
                                  local.get 11
                                  local.set 6
                                  br 14 (;@1;)
                                end
                                local.get 10
                                local.get 6
                                i32.add
                                local.set 10
                              end
                              local.get 5
                              i32.const 2
                              i32.add
                              local.set 0
                              local.get 11
                              local.get 1
                              i32.const 11
                              i32.shl
                              i32.const 16384
                              i32.and
                              i32.sub
                              local.get 5
                              i32.load8_u offset=1
                              i32.const 6
                              i32.shl
                              local.get 5
                              i32.load8_u
                              i32.const 2
                              i32.shr_u
                              i32.or
                              i32.sub
                              local.tee 6
                              local.get 11
                              i32.eq
                              br_if 7 (;@6;)
                              local.get 6
                              i32.const -16384
                              i32.add
                              local.set 9
                            end
                            local.get 9
                            local.get 2
                            i32.lt_u
                            br_if 7 (;@5;)
                            local.get 9
                            local.get 11
                            i32.ge_u
                            br_if 7 (;@5;)
                            local.get 8
                            local.get 11
                            i32.sub
                            local.get 10
                            i32.const 2
                            i32.add
                            i32.lt_u
                            br_if 2 (;@10;)
                          end
                          local.get 11
                          local.get 9
                          i32.load8_u
                          i32.store8
                          local.get 11
                          local.get 9
                          i32.load8_u offset=1
                          i32.store8 offset=1
                          local.get 11
                          i32.const 2
                          i32.add
                          local.set 6
                          local.get 9
                          i32.const 2
                          i32.add
                          local.set 1
                          loop  ;; label = @12
                            local.get 6
                            local.get 1
                            i32.load8_u
                            i32.store8
                            local.get 6
                            i32.const 1
                            i32.add
                            local.set 6
                            local.get 1
                            i32.const 1
                            i32.add
                            local.set 1
                            local.get 10
                            i32.const -1
                            i32.add
                            local.tee 10
                            br_if 0 (;@12;)
                            br 5 (;@7;)
                          end
                        end
                        local.get 5
                        i32.load8_u
                        i32.const 2
                        i32.shl
                        i32.const -1
                        i32.xor
                        local.get 11
                        local.get 1
                        i32.const 2
                        i32.shr_u
                        i32.sub
                        i32.add
                        local.tee 1
                        local.get 2
                        i32.lt_u
                        br_if 5 (;@5;)
                        local.get 1
                        local.get 11
                        i32.ge_u
                        br_if 5 (;@5;)
                        local.get 8
                        local.get 11
                        i32.sub
                        i32.const 2
                        i32.ge_u
                        br_if 1 (;@9;)
                      end
                      local.get 11
                      local.set 6
                      br 7 (;@2;)
                    end
                    local.get 5
                    i32.const 1
                    i32.add
                    local.set 0
                    local.get 11
                    local.get 1
                    i32.load8_u
                    i32.store8
                    local.get 11
                    i32.const 2
                    i32.add
                    local.set 6
                    local.get 11
                    i32.const 1
                    i32.add
                    local.set 10
                  end
                  local.get 10
                  local.get 1
                  i32.load8_u offset=1
                  i32.store8
                end
                local.get 0
                i32.const -2
                i32.add
                i32.load8_u
                i32.const 3
                i32.and
                local.tee 10
                i32.eqz
                br_if 2 (;@4;)
                i32.const 2
                local.set 1
                br 3 (;@3;)
              end
              local.get 3
              local.get 11
              local.get 2
              i32.sub
              i32.store
              i32.const 0
              i32.const -8
              i32.const -4
              local.get 0
              local.get 7
              i32.lt_u
              select
              local.get 0
              local.get 7
              i32.eq
              select
              return
            end
            local.get 3
            local.get 11
            local.get 2
            i32.sub
            i32.store
            i32.const -6
            return
          end
          i32.const 0
          local.set 1
          br 0 (;@3;)
        end
      end
      local.get 3
      local.get 6
      local.get 2
      i32.sub
      i32.store
      i32.const -5
      return
    end
    local.get 3
    local.get 6
    local.get 2
    i32.sub
    i32.store
    i32.const -4)
  (func $entry (type 2) (result i32)
    (local i32 i32)
    i32.const 0
    i32.const 65536
    call 4
    i32.store offset=44
    i32.const 0
    i32.const 268435456
    call 4
    i32.store offset=48
    i32.const 0
    i32.const 285212739
    call 4
    i32.store offset=52
    i32.const 0
    i32.load offset=48
    i32.const 0
    i32.const 268435456
    call 3
    drop
    i32.const 56
    i32.const 66
    call 5
    local.set 0
    i32.const 0
    i32.load offset=48
    i32.const 1
    i32.const 268435456
    local.get 0
    call 6
    local.set 1
    local.get 0
    call 7
    drop
    i32.const 0
    i32.load offset=48
    local.get 1
    i32.const 0
    i32.load offset=52
    i32.const 40
    i32.const 0
    i32.load offset=44
    call $lzo1x_1_compress
    drop
    i32.const 69
    i32.const 83
    call 5
    local.set 0
    i32.const 0
    i32.load offset=52
    i32.const 1
    i32.const 0
    i32.load offset=40
    local.get 0
    call 8
    drop
    local.get 0
    call 7
    drop
    i32.const 0
    i32.load offset=40)
  (memory (;0;) 1)
  (export "main" (func $entry))
  (data $lzo_copyright_ (i32.const 0) "2.10\00")
  (data $lzo_version_string_ (i32.const 5) "2.10\00")
  (data $lzo_version_date_ (i32.const 10) "Mar 01 2017\00")
  (data $_lzo_config_check.u (i32.const 24) "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00")
  (data $entry.out_len (i32.const 40) "\00\00\00\00")
  (data $wrkmem (i32.const 44) "\00\00\00\00")
  (data $in (i32.const 48) "\00\00\00\00")
  (data $out (i32.const 52) "\00\00\00\00")
  (data $.L.str (i32.const 56) "input.txt\00")
  (data $.L.str.1 (i32.const 66) "rb\00")
  (data $.L.str.2 (i32.const 69) "input.txt.lzo\00")
  (data $.L.str.3 (i32.const 83) "wb\00"))
