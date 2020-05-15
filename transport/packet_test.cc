//
// Created by Chengke Wong on 2020/4/9.
//

#include "gtest/gtest.h"
#include "util/string_raw.h"
#include "transport/packet_header.h"

class PacketTest : public ::testing::Test {
protected:
    PacketTest() = default;

    ~PacketTest() override = default;

    void SetUp() override {
    }

    void TearDown() override {
    };

    // https://quicwg.org/base-drafts/draft-ietf-quic-tls.html#name-client-initial
    // The client sends an Initial packet. The unprotected payload of this
    // packet contains the following CRYPTO frame, plus enough PADDING frames
    // to make a 1162 byte payload:
    String initial_packet = String::from_hex(
        "c0ff00001b088394c8f03e5157080000 449e3b343aa8535064a4268a0d9d7b1c\n"
        "9d250ae355162276e9b1e3011ef6bbc0 ab48ad5bcc2681e953857ca62becd752\n"
        "4daac473e68d7405fbba4e9ee616c870 38bdbe908c06d9605d9ac49030359eec\n"
        "b1d05a14e117db8cede2bb09d0dbbfee 271cb374d8f10abec82d0f59a1dee29f\n"
        "e95638ed8dd41da07487468791b719c5 5c46968eb3b54680037102a28e53dc1d\n"
        "12903db0af5821794b41c4a93357fa59 ce69cfe7f6bdfa629eef78616447e1d6\n"
        "11c4baf71bf33febcb03137c2c75d253 17d3e13b684370f668411c0f00304b50\n"
        "1c8fd422bd9b9ad81d643b20da89ca05 25d24d2b142041cae0af205092e43008\n"
        "0cd8559ea4c5c6e4fa3f66082b7d303e 52ce0162baa958532b0bbc2bc785681f\n"
        "cf37485dff6595e01e739c8ac9efba31 b985d5f656cc092432d781db95221724\n"
        "87641c4d3ab8ece01e39bc85b1543661 4775a98ba8fa12d46f9b35e2a55eb72d\n"
        "7f85181a366663387ddc20551807e007 673bd7e26bf9b29b5ab10a1ca87cbb7a\n"
        "d97e99eb66959c2a9bc3cbde4707ff77 20b110fa95354674e395812e47a0ae53\n"
        "b464dcb2d1f345df360dc227270c7506 76f6724eb479f0d2fbb6124429990457\n"
        "ac6c9167f40aab739998f38b9eccb24f d47c8410131bf65a52af841275d5b3d1\n"
        "880b197df2b5dea3e6de56ebce3ffb6e 9277a82082f8d9677a6767089b671ebd\n"
        "244c214f0bde95c2beb02cd1172d58bd f39dce56ff68eb35ab39b49b4eac7c81\n"
        "5ea60451d6e6ab82119118df02a58684 4a9ffe162ba006d0669ef57668cab38b\n"
        "62f71a2523a084852cd1d079b3658dc2 f3e87949b550bab3e177cfc49ed190df\n"
        "f0630e43077c30de8f6ae081537f1e83 da537da980afa668e7b7fb25301cf741\n"
        "524be3c49884b42821f17552fbd1931a 813017b6b6590a41ea18b6ba49cd48a4\n"
        "40bd9a3346a7623fb4ba34a3ee571e3c 731f35a7a3cf25b551a680fa68763507\n"
        "b7fde3aaf023c50b9d22da6876ba337e b5e9dd9ec3daf970242b6c5aab3aa4b2\n"
        "96ad8b9f6832f686ef70fa938b31b4e5 ddd7364442d3ea72e73d668fb0937796\n"
        "f462923a81a47e1cee7426ff6d922126 9b5a62ec03d6ec94d12606cb485560ba\n"
        "b574816009e96504249385bb61a819be 04f62c2066214d8360a2022beb316240\n"
        "b6c7d78bbe56c13082e0ca272661210a bf020bf3b5783f1426436cf9ff418405\n"
        "93a5d0638d32fc51c5c65ff291a3a7a5 2fd6775e623a4439cc08dd25582febc9\n"
        "44ef92d8dbd329c91de3e9c9582e41f1 7f3d186f104ad3f90995116c682a2a14\n"
        "a3b4b1f547c335f0be710fc9fc03e0e5 87b8cda31ce65b969878a4ad4283e6d5\n"
        "b0373f43da86e9e0ffe1ae0fddd35162 55bd74566f36a38703d5f34249ded1f6\n"
        "6b3d9b45b9af2ccfefe984e13376b1b2 c6404aa48c8026132343da3f3a33659e\n"
        "c1b3e95080540b28b7f3fcd35fa5d843 b579a84c089121a60d8c1754915c344e\n"
        "eaf45a9bf27dc0c1e784161691220913 13eb0e87555abd706626e557fc36a04f\n"
        "cd191a58829104d6075c5594f627ca50 6bf181daec940f4a4f3af0074eee89da\n"
        "acde6758312622d4fa675b39f728e062 d2bee680d8f41a597c262648bb18bcfc\n"
        "13c8b3d97b1a77b2ac3af745d61a34cc 4709865bac824a94bb19058015e4e42d\n"
        "38d3b779d72edc00c5cd088eff802b05");

    // https://quicwg.org/base-drafts/draft-ietf-quic-tls.html#name-server-initial
    // The server sends the following payload in response, including an ACK
    // frame, a CRYPTO frame, and no PADDING frames
    String server_initial = String::from_hex(
        "c9ff00001b0008f067a5502a4262b500 4074168bf22b7002596f99ae67abf65a\n"
        "5852f54f58c37c808682e2e40492d8a3 899fb04fc0afe9aabc8767b18a0aa493\n"
        "537426373b48d502214dd856d63b78ce e37bc664b3fe86d487ac7a77c53038a3\n"
        "cd32f0b5004d9f5754c4f7f2d1f35cf3 f7116351c92bd8c3a9528d2b6aca20f0\n"
        "8047d9f017f0");

    String client_hp =
        String::from_hex("a980b8b4fb7d9fbc13e814c23164253d");
};

TEST_F(PacketTest, DecodeHeader) {
    StringReader reader(initial_packet);
    PacketHeader header =
        PacketHeader::from_reader(reader);

    EXPECT_EQ(header.token.value().size(), 0);
    EXPECT_EQ(header.length, 1182);
    EXPECT_EQ(header.scid.to_hex(), "");
    EXPECT_EQ(header.dcid.to_hex(), "8394c8f03e515708");
}

TEST_F(PacketTest, DecodeHeader2) {
    StringReader reader(server_initial);
    PacketHeader header =
        PacketHeader::from_reader(reader);

    EXPECT_EQ(header.token.value().size(), 0);
    EXPECT_EQ(header.header_length, 18);
    EXPECT_EQ(header.length, 116);
    EXPECT_EQ(header.scid.to_hex(), "f067a5502a4262b5");
    EXPECT_EQ(header.dcid.to_hex(), "");
}

TEST_F(PacketTest, DecryptHeader) {
    StringReader reader(initial_packet);
    PacketHeader header =
        PacketHeader::from_reader(reader);
    String hp = String::from_hex("a980b8b4fb7d9fbc13e814c23164253d");
    header.decrypt(hp, reader);
    EXPECT_EQ(header.pkt_number.value, 2);
    EXPECT_EQ(header.payload_offset(), 22);
}

TEST_F(PacketTest, DecryptHeader2) {
    StringReader reader(server_initial);
    PacketHeader header =
        PacketHeader::from_reader(reader);
    String hp = String::from_hex("a8ed82e6664f865aedf6106943f95fb8");
    header.decrypt(hp, reader);
    EXPECT_EQ(header.pkt_number.value, 1);
    EXPECT_EQ(header.pkt_number_len, 2);
    EXPECT_EQ(header.payload_offset(), 20);
}
