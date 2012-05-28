

class Node(object):


    def dump(self, stream, indent=0):
        for attr, child in self.children:
            print "%s%s: %r" % ("    " * indent, attr, child)
            if hasattr(child, 'dump'):
                child.dump(stream, indent + 1)
        return


    def graphviz(self, stream):
        print >>stream, '    n%d[label="%r"];' % (id(self), self)
        
        for attr, child in self.children:
            if hasattr(child, 'graphviz'):
                child.graphviz(stream)
            else:
                print >>stream, '    n%d[label="%r"];' % (id(child), child)
        
        print >>stream
        for attr, child in self.children:
            print >>stream, '    n%d->n%d[label="%s"];' % (
                id(self), id(child), attr)

        return
