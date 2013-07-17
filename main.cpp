#include <rct/Path.h>
#include <rct/String.h>
#include <rct/Log.h>

struct Chunk {
    enum Type {
        Added,
        Removed,
        Unchanged
    };
    Chunk()
        : type(Unchanged), start(-1), size(-1)
    {}

    Type type;
    int start, size;
};

static bool compare(const char *lstr, int lsize, const char *rstr, int rsize, int fuzz)
{
    if (lsize < rsize)
        return compare(rstr, rsize, lstr, lsize, fuzz);

    const int diff = lsize - rsize;
    assert(diff >= 0);
    if (diff > fuzz)
        return false;

    int i = 0;
    while (i < rsize) {
        if (*lstr != *rstr) {
            if (fuzz > 0) {
                --fuzz;
                ++lstr;
            } else {
                return false;
            }
        } else {
            ++lstr;
            ++rstr;
            ++i;
        }
    }
    return true;
}

static inline int same(const char *l, const char *r, int fuzz = 0)
{
    int ret = 0;
    while (*l && *l == *r) {
        ++ret;
        ++r;
        ++l;
    }
    return ret;
}

enum MatchType {
    Match,
    NoMatch,
    FuzzMatch
};

struct Token
{
    const char *string;
    int length;
    enum Type {
        Word,
        Boundary
    } type;

    MatchType match(const Token &other) const
    {
        if (type != other.type)
            return NoMatch;
        if (type == Boundary)
            return length == other.length ? Match : FuzzMatch;

        if (length == other.length)
            return strncmp(string, other.string, length) ? NoMatch : Match;

        const bool leftLarger = length > other.length;
        if (length < other.length) {
            return compare(string, length, other.string, other.length, std::min(3, length - other.length)) ? FuzzMatch : NoMatch;
        } else {
            return compare(other.string, other.length, string, length, std::min(3, other.length - length))  ? FuzzMatch : NoMatch;
        }
    }
};

List<Token> tokenizeString(const String &string)
{
    int last = 0;
    int idx = 0;
    const char *const cstr = string.constData();
    const char *str = cstr;
    enum State {
        Word,
        Boundary,
        Symbol,
        Unset
    } state = Unset;

    List<Token> tokens;

    while (*str) {
        State s;
        if (isspace(*str)) {
            s = Boundary;
        } else if (isalnum(*str)) {
            s = Word;
        } else {
            s = Symbol;
        }
        if (state == Unset) {
            state = s;
        } else if (s != state) {
            const Token token = { cstr + last, idx - last, state == Boundary ? Token::Boundary : Token::Word };
            tokens.append(token);
            // error() << "Adding token" << last << (idx - last) << state;
            last = idx;
            state = s;
        }
        ++str;
        ++idx;
    }
    if (last != idx) {
        const Token token = { cstr + last, idx - last, state == Boundary ? Token::Boundary : Token::Word };
        // error() << "Adding token" << last << (idx - last) << state;
        tokens.append(token);
    }
    return tokens;
}

static MatchType compare(const List<Token> &left, const List<Token> &right, int fuzz)
{
    if (left.size() < right.size())
        return compare(right, left, fuzz);

    const int diff = left.size() - right.size();
    assert(diff >= 0);
    if (diff > fuzz)
        return NoMatch;

    int lidx = 0, ridx = 0;
    MatchType ret = Match;
    while (lidx < left.size()) {
        switch (left.at(lidx).match(right.at(ridx))) {
        case FuzzMatch:
            ret = FuzzMatch;
            break;
        case NoMatch:
            if (fuzz > 0) {
                --fuzz;
                ++lidx;
                continue;
            }
            return NoMatch;
        case Match:
            break;
        }
        ++lidx;
        ++ridx;
    }
    return ret;
}


MatchType match(const String &left, const String &right)
{
    if (left.isEmpty())
        return right.isEmpty() ? Match : NoMatch;

    const int count = same(left.constData(), right.constData());
    const int lsize = left.size();
    const int rsize = right.size();

    if (count == lsize || count == rsize)
        return lsize == rsize ? Match : FuzzMatch;
    const List<Token> ltokens = tokenizeString(left);
    const List<Token> rtokens = tokenizeString(right);

    if (ltokens.size() >= rtokens.size()) {
        return compare(rtokens, ltokens, std::min(1, ltokens.size() - rtokens.size()));
    } else {
        return compare(rtokens, ltokens, std::min(1, rtokens.size() - ltokens.size()));
    }
}

static List<Chunk> diff(const String &prev, const String &cur)
{
    List<Chunk> chunks;
    List<String> prevLines = prev.split('\n');
    List<String> curLines = cur.split('\n');
    // const char *l = prev.c_str();
    // const char *r = cur.c_str();
    return chunks;
}

int main(int argc, char **argv)
{
    if (argc == 2) {
        const String str(argv[1]);
        List<Token> tokens = tokenizeString(str);
        for (List<Token>::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
            error() << String(it->string, it->length) << (it->type == Token::Boundary ? "boundary" : "word");
        }

        return 0;
    }
    if (argc != 3) {
        printf("[%s:%d]: if (argc != 3) {\n", __func__, __LINE__); fflush(stdout);
        return 1;
    }
    // error() << "compared" << compare(argv[1], strlen(argv[1]), argv[2], strlen(argv[2]), 2);
    // return 0;

    const Path prev(argv[1]);
    const Path cur(argv[2]);
    List<Chunk> chunks = diff(prev.readAll(), cur.readAll());
    for (List<Chunk>::const_iterator it = chunks.begin(); it != chunks.end(); ++it) {
        error() << it->type << it->start << it->size;
    }

    return 0;
}
