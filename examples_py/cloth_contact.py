import sys
sys.path.append("../module_py")
import dfm2


def example1():
  cad = dfm2.Cad2D(list_xy=[-1,-1, +1,-1, +1,+1, -1,+1.0])
  mesh = cad.mesh(0.05)
  fem = dfm2.FEM_Cloth(mesh)
  npIdP = cad.points_edge([2], mesh.np_pos)
  fem.ls.bc[npIdP,0:3] = 1
  fem.sdf.list_sdf.append( dfm2.SDF_Sphere(0.55,[0,+0.5,-1.0],True) )
  ####
  mesh2 = dfm2.Mesh(np_pos=fem.vec_val,np_elm=mesh.np_elm)
  axis = dfm2.AxisXYZ(1.0)
  dfm2.winDraw3d([fem,mesh2,axis,fem.sdf])


if __name__ == "__main__":
  example1()