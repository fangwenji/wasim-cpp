package mycpu

import chisel3._
import chisel3.util._
import chiseltest._
import chisel3.experimental.BundleLiterals._
import chiseltest.formal._
import chiseltest.formal.BoundedCheck
import utest._
//import  Constants._

class partial_product extends Module {
  val io = IO(new Bundle{
    val y = Input(UInt(3.W))
    val x = Input(UInt(64.W))
    val c = Output(UInt(1.W))
    val p = Output(UInt(64.W))
  })
  
  io.p := MuxLookup(io.y, 0.U, Array(
    0.U -> 0.U,
    1.U -> io.x,
    2.U -> io.x,
    3.U -> (io.x << 1),
    4.U -> ~(io.x << 1),
    5.U -> ~io.x, 
    6.U -> ~io.x,
    7.U -> 0.U
  ))
  io.c := MuxLookup(io.y, 0.U, Array(
    4.U -> 1.U,
    5.U -> 1.U,
    6.U -> 1.U
  ))
}

class  Mul extends Module{
    val io = IO(new Bundle{
        val mul_valid = Input(Bool())
        val flush = Input(Bool())
        val mulw = Input(Bool())
        val mul_signed = Input(UInt(2.W))
        val multiplicand = Input(UInt(64.W))
        val multiplier = Input(UInt(64.W))
        val mul_ready = Output(Bool())
        val out_valid = Output(Bool())
        val out_ready = Input(Bool())
        val result_hi = Output(UInt(32.W))
        val result_lo = Output(UInt(32.W))
    })
    val sign = Wire(UInt(1.W))
    sign := io.multiplicand(63) ^ io.multiplier(63)

    val res = RegInit(0.U(64.W))
    val idle :: mul_play :: return_val :: Nil = Enum(3)
    val state = RegInit(idle)

    val src1 = RegInit(0.U(128.W))
    val src2 = RegInit(0.U(65.W))

    val real_cand = Wire(UInt(128.W))
    val real_er = Wire(UInt(64.W))

    val src1_32 = Wire(UInt(64.W))
    val src2_32 = Wire(UInt(64.W))
    src1_32 := Mux(io.mulw,Cat(Fill(32,io.multiplicand(31)),io.multiplicand(31,0)),io.multiplicand)
    src2_32 := Mux(io.mulw,Cat(Fill(32,io.multiplier(31)),io.multiplier(31,0)),io.multiplier)

    real_cand := Mux(io.mul_signed===3.U||io.mul_signed===2.U, Cat(Fill(64,src1_32(63)),src1_32), src1_32)
    real_er := Cat(src2_32,0.U(1.W))
    
    val booth_partial = Module(new partial_product())
    booth_partial.io.y := src2(2,0)
    booth_partial.io.x :=  src1

    switch(state){
        is(idle){
            when(~io.flush && io.mul_valid){
                state := mul_play
                src1 := real_cand
                src2 := real_er
                res := 0.U
            }
        }
        is(mul_play){
            when(io.flush){
                state := idle
            }.otherwise{
                when(src2=/=0.U){
                    res := res + booth_partial.io.p + booth_partial.io.c
                    src2 := src2 >> 2
                    src1 := src1 << 2
                }.otherwise{
                    when(io.out_ready){
                        state := idle
                    }
                }

            }
        }
    }
    when(state===idle){
        io.mul_ready := true.B
        io.out_valid := false.B
        io.result_hi := 0.U
        io.result_lo := 0.U
    }.elsewhen(state===mul_play){
        when(src2=/=0.U||io.flush){
            io.mul_ready := false.B
            io.out_valid := false.B
            io.result_hi := 0.U
            io.result_lo := 0.U
        }.otherwise{
            io.result_hi := Cat(sign,res(62,32))
            io.result_lo := res(31,0)
            io.out_valid := true.B
            io.mul_ready := false.B
        }
    }.otherwise{
        io.mul_ready := false.B
        io.out_valid := false.B
        io.result_hi := 0.U
        io.result_lo := 0.U
    }
    //printf("state:%d  src1_32:%x  src2_32:%x src1:%x  src2:%x\n",state,src1_32,src2_32,src1,src2)
}

