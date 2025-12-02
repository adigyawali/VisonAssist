"use client";
import React, { useState } from "react";

interface DistanceBarsProps {
  left: number;
  front: number;
  right:number;
}

class Queue {
  private items: number[] = [];
  private length:number;

  constructor(avgLength:number) {
    this.length=avgLength;
  }

  enqueue(item: number): void {
    if(this.size()+1 > this.length) this.dequeue();
    this.items.push(item);
  }

  dequeue(): number | undefined {
    return this.items.shift();
  }

  peek(): number | undefined {
    return this.items[0];
  }

  isEmpty(): boolean {
    return this.items.length === 0;
  }

  size(): number {
    return this.items.length;
  }

  avg(): number {
    let sum = 0
    if (this.size()==0) return 0;
    this.items.forEach(element => {
      sum += element;
    });
    return sum/this.size();
  }
}

export default function DistanceBars({ left, front, right }: DistanceBarsProps) {
  // Clamp distances to 0–2 m
  const clamp = (val: number) => Math.min(Math.max(val, 0), 200);
  let leftHeight=0;
  let rightHeight=0;
  let leftDelay=0;
  let rightDelay=0;

  let rightAvg:Queue=new Queue(10);
  let leftAvg: Queue = new Queue(10);

  const getColor = (height: number) => {
    if (height > 75) return "bg-red-500";
    if (height > 35) return "bg-yellow-400";
    return "bg-green-400";
  };

  const getLeft = () => {
    leftAvg.enqueue(left);
    if (leftAvg.avg() > 0) {
      leftHeight = (1 - clamp(leftAvg.avg()) / 200) * 100;
      leftDelay=0;
    }
    else if(leftDelay>=5) return 0;
    else leftDelay++;
    return leftHeight;
  }

  const getRight = () => {
    rightAvg.enqueue(right);
    if (rightAvg.avg() > 0) {
      rightHeight=(1 - clamp(rightAvg.avg()) / 200) * 100;
      rightDelay=0;
    }
    else if (rightDelay>=5) return 0;
    else rightDelay++;
    return rightHeight;
  }

  return (
    <div className="flex items-end justify-center gap-16 h-64 w-96 bg-neutral-700/60 rounded-xl p-6 shadow-inner">

      {/* Left bar */}
      <div className="flex flex-col items-center justify-end h-full">
        <div
          className={`w-14 transition-all duration-500 rounded-t-md ${getColor(getLeft())}`}
          style={{ height: `${getLeft()}%` }}
        ></div>
        <span className="mt-2 text-sm font-medium text-gray-300">Obstacle</span>
      </div>
      {/* Right bar */}
      <div className="flex flex-col items-center justify-end h-full">
        <div
          className={`w-14 transition-all duration-500 rounded-t-md ${getColor(getLeft())}`}
          style={{ height: `${getRight()}%` }}
        ></div>
        <span className="mt-2 text-sm font-medium text-gray-300">Trench</span>
      </div>
    </div>
  );
}
