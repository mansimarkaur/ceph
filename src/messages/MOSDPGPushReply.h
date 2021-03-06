// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2013 Inktank Storage, Inc.
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */

#ifndef MOSDPGPUSHREPLY_H
#define MOSDPGPUSHREPLY_H

#include "MOSDFastDispatchOp.h"

class MOSDPGPushReply : public MOSDFastDispatchOp {
  static const int HEAD_VERSION = 2;
  static const int COMPAT_VERSION = 1;

public:
  pg_shard_t from;
  spg_t pgid;
  epoch_t map_epoch;
  vector<PushReplyOp> replies;
  uint64_t cost;

  epoch_t get_map_epoch() const override {
    return map_epoch;
  }
  spg_t get_spg() const override {
    return pgid;
  }

  MOSDPGPushReply()
    : MOSDFastDispatchOp(MSG_OSD_PG_PUSH_REPLY, HEAD_VERSION, COMPAT_VERSION),
      cost(0)
    {}

  void compute_cost(CephContext *cct) {
    cost = 0;
    for (vector<PushReplyOp>::iterator i = replies.begin();
	 i != replies.end();
	 ++i) {
      cost += i->cost(cct);
    }
  }

  int get_cost() const override {
    return cost;
  }

  void decode_payload() override {
    bufferlist::iterator p = payload.begin();
    ::decode(pgid.pgid, p);
    ::decode(map_epoch, p);
    ::decode(replies, p);
    ::decode(cost, p);

    if (header.version >= 2) {
      ::decode(pgid.shard, p);
      ::decode(from, p);
    } else {
      pgid.shard = shard_id_t::NO_SHARD;
      from = pg_shard_t(get_source().num(), shard_id_t::NO_SHARD);
    }
  }

  void encode_payload(uint64_t features) override {
    ::encode(pgid.pgid, payload);
    ::encode(map_epoch, payload);
    ::encode(replies, payload);
    ::encode(cost, payload);
    ::encode(pgid.shard, payload);
    ::encode(from, payload);
  }

  void print(ostream& out) const override {
    out << "MOSDPGPushReply(" << pgid
	<< " " << map_epoch
	<< " " << replies;
    out << ")";
  }

  const char *get_type_name() const override { return "MOSDPGPushReply"; }
};

#endif
